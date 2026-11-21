#include "./FlexSolver.hpp"

#include <algorithm>
#include <cmath>

namespace w32oop::ui::detail {
	namespace {

		constexpr double kEpsilon = 1e-6;

		// 求解过程中的单项目工作数据。全部使用 double，最后统一取整，
		// 这样相邻项目的边界是「同一个数取整」，不会出现 1px 缝或重叠。
		struct WorkItem {
			FlexItemSpec* spec = nullptr;

			int minMain = 0;
			int maxMain = INT_MAX;
			int minCross = 0;
			int maxCross = INT_MAX;

			int marginMainStart = 0;
			int marginMainEnd = 0;
			int marginCrossStart = 0;
			int marginCrossEnd = 0;

			double base = 0.0;          // flex base size（未夹取）
			double hypothetical = 0.0;  // 夹取后的 base，用于分行与自由空间
			double target = 0.0;        // 弹性计算后的主轴尺寸
			double cross = 0.0;         // 交叉轴尺寸
			double mainStart = 0.0;     // 主轴起点（逻辑坐标，reverse 时以主起点边为 0）
			double mainEnd = 0.0;
			double crossStart = 0.0;
			double violation = 0.0;     // 夹取前后的差值，仅用于冻结判定

			bool frozen = false;
			bool crossIsAuto = false;
			Align align = Align::Start;
		};

		struct Line {
			std::size_t begin = 0;
			std::size_t end = 0;
			double cross = 0.0;
			double offset = 0.0;
		};

		int clamp_int(int value, int lo, int hi) {
			if (hi < lo) hi = lo;
			if (value < lo) return lo;
			if (value > hi) return hi;
			return value;
		}

		double clamp_double(double value, double lo, double hi) {
			if (hi < lo) hi = lo;
			if (value < lo) return lo;
			if (value > hi) return hi;
			return value;
		}

		Align resolve_align(const FlexItemSpec& spec, Align container_align) {
			Align align = spec.alignSelf == Align::Auto ? container_align : spec.alignSelf;
			return align == Align::Auto ? Align::Start : align;
		}

		double outer_main(const WorkItem& item, bool use_target) {
			return (use_target ? item.target : item.base)
				+ item.marginMainStart + item.marginMainEnd;
		}

		// CSS 9.7「Resolve Flexible Lengths」的冻结循环。
		// 输入：base / hypothetical / grow / shrink / min / max / 主轴外边距。
		// 输出：item.target 为最终主轴尺寸。
		void resolve_flexible_lengths(
			std::vector<WorkItem>& work,
			std::size_t begin,
			std::size_t end,
			double inner_main,
			double main_gap)
		{
			const std::size_t count = end - begin;
			const double total_gap = count > 1 ? main_gap * static_cast<double>(count - 1) : 0.0;

			for (std::size_t i = begin; i < end; ++i) {
				WorkItem& item = work[i];
				item.target = item.base;
				item.frozen = false;

				const float grow = item.spec->grow;
				const float shrink = item.spec->shrink;
				if (grow <= 0.0f && shrink <= 0.0f) {
					// 完全不参与伸缩：冻结在假想尺寸（已按 min/max 夹取）。
					item.target = item.hypothetical;
					item.frozen = true;
				}
				else if (grow <= 0.0f && item.base > item.hypothetical + kEpsilon) {
					item.target = item.hypothetical;
					item.frozen = true;
				}
				else if (shrink <= 0.0f && item.base < item.hypothetical - kEpsilon) {
					item.target = item.hypothetical;
					item.frozen = true;
				}
			}

			// 主轴无界：不做任何伸缩，按内容排。
			if (inner_main == LayoutSize::Unbounded) {
				for (std::size_t i = begin; i < end; ++i) work[i].frozen = true;
				return;
			}

			auto compute_free_space = [&]() {
				double free_space = inner_main - total_gap;
				for (std::size_t i = begin; i < end; ++i) {
					free_space -= outer_main(work[i], work[i].frozen);
				}
				return free_space;
			};

			double free_space = compute_free_space();

			// 冻结循环最多跑「项目数 + 2」次就能收敛；给足 64 次防御性上限。
			for (int iteration = 0; iteration < 64; ++iteration) {
				bool has_unfrozen = false;
				for (std::size_t i = begin; i < end; ++i) {
					if (!work[i].frozen) { has_unfrozen = true; break; }
				}
				if (!has_unfrozen) return;

				if (free_space > kEpsilon) {
					double sum_grow = 0.0;
					for (std::size_t i = begin; i < end; ++i) {
						if (!work[i].frozen) sum_grow += (std::max)(0.0f, work[i].spec->grow);
					}
					if (sum_grow <= kEpsilon) {
						for (std::size_t i = begin; i < end; ++i) work[i].frozen = true;
						return;
					}
					double distributable = free_space;
					if (sum_grow < 1.0) distributable = free_space * sum_grow;
					for (std::size_t i = begin; i < end; ++i) {
						WorkItem& item = work[i];
						if (item.frozen) continue;
						const double factor = (std::max)(0.0f, item.spec->grow) / sum_grow;
						item.target = item.base + distributable * factor;
					}
				}
				else if (free_space < -kEpsilon) {
					double sum_scaled_shrink = 0.0;
					for (std::size_t i = begin; i < end; ++i) {
						if (work[i].frozen) continue;
						sum_scaled_shrink += (std::max)(0.0f, work[i].spec->shrink) * work[i].base;
					}
					if (sum_scaled_shrink <= kEpsilon) {
						for (std::size_t i = begin; i < end; ++i) work[i].frozen = true;
						return;
					}
					for (std::size_t i = begin; i < end; ++i) {
						WorkItem& item = work[i];
						if (item.frozen) continue;
						const double weight = (std::max)(0.0f, item.spec->shrink) * item.base;
						item.target = item.base + free_space * (weight / sum_scaled_shrink);
					}
				}
				else {
					for (std::size_t i = begin; i < end; ++i) work[i].frozen = true;
					return;
				}

				// 夹取到 min/max，并记录违反量，决定冻结哪一批。
				double total_violation = 0.0;
				for (std::size_t i = begin; i < end; ++i) {
					WorkItem& item = work[i];
					if (item.frozen) continue;
					const double unclamped = item.target;
					item.target = clamp_double(unclamped, item.minMain, item.maxMain);
					item.violation = item.target - unclamped;
					total_violation += item.violation;
				}

				if (std::fabs(total_violation) <= kEpsilon) {
					for (std::size_t i = begin; i < end; ++i) work[i].frozen = true;
					return;
				}

				const bool freeze_positive = total_violation > 0.0;
				for (std::size_t i = begin; i < end; ++i) {
					WorkItem& item = work[i];
					if (item.frozen) continue;
					if (std::fabs(item.violation) <= kEpsilon) continue;
					if ((item.violation > 0.0) == freeze_positive) item.frozen = true;
				}

				free_space = compute_free_space();
			}
		}
	}

	FlexResult solve_flex(
		const FlexSpec& spec,
		std::vector<FlexItemSpec>& items,
		LayoutSize available)
	{
		FlexResult result;
		result.lineCount = 0;

		const bool horizontal = is_horizontal(spec.direction);
		const bool reverse = is_reverse(spec.direction);
		const int pad_left = spec.padding.left;
		const int pad_top = spec.padding.top;

		int inner_main = 0;
		int inner_cross = 0;
		if (horizontal) {
			inner_main = available.isWidthUnbounded()
				? LayoutSize::Unbounded
				: (std::max)(0, available.width - spec.padding.horizontal());
			inner_cross = available.isHeightUnbounded()
				? LayoutSize::Unbounded
				: (std::max)(0, available.height - spec.padding.vertical());
		}
		else {
			inner_main = available.isHeightUnbounded()
				? LayoutSize::Unbounded
				: (std::max)(0, available.height - spec.padding.vertical());
			inner_cross = available.isWidthUnbounded()
				? LayoutSize::Unbounded
				: (std::max)(0, available.width - spec.padding.horizontal());
		}

		const int main_gap = horizontal ? spec.columnGap : spec.rowGap;
		const int cross_gap = horizontal ? spec.rowGap : spec.columnGap;

		// 嵌套布局的内容尺寸：主轴取 max-content，交叉轴用容器内交叉尺寸。
		// 这样 column 容器里的子容器能拿到「给定宽度下的高度」。
		const LayoutSize measure_available = horizontal
			? LayoutSize{ LayoutSize::Unbounded, inner_cross }
			: LayoutSize{ inner_cross, LayoutSize::Unbounded };

		std::vector<WorkItem> work(items.size());
		for (std::size_t i = 0; i < items.size(); ++i) {
			FlexItemSpec& item = items[i];
			WorkItem& w = work[i];
			w.spec = &item;

			if (item.maxWidth < item.minWidth) item.maxWidth = item.minWidth;
			if (item.maxHeight < item.minHeight) item.maxHeight = item.minHeight;
			if (item.minWidth < 0) item.minWidth = 0;
			if (item.minHeight < 0) item.minHeight = 0;

			LayoutSize content = item.content;
			if (item.contentMeasure) content = item.contentMeasure(measure_available);
			content.width = (std::max)(0, content.width);
			content.height = (std::max)(0, content.height);
			const int content_main = horizontal ? content.width : content.height;
			const int content_cross = horizontal ? content.height : content.width;

			const Length& main_length = horizontal ? item.width : item.height;
			const Length& cross_length = horizontal ? item.height : item.width;

			w.minMain = horizontal ? item.minWidth : item.minHeight;
			w.maxMain = horizontal ? item.maxWidth : item.maxHeight;
			w.minCross = horizontal ? item.minHeight : item.minWidth;
			w.maxCross = horizontal ? item.maxHeight : item.maxWidth;

			if (horizontal) {
				w.marginMainStart = reverse ? item.margin.right : item.margin.left;
				w.marginMainEnd = reverse ? item.margin.left : item.margin.right;
				w.marginCrossStart = item.margin.top;
				w.marginCrossEnd = item.margin.bottom;
			}
			else {
				w.marginMainStart = reverse ? item.margin.bottom : item.margin.top;
				w.marginMainEnd = reverse ? item.margin.top : item.margin.bottom;
				w.marginCrossStart = item.margin.left;
				w.marginCrossEnd = item.margin.right;
			}

			double base = 0.0;
			if (!item.basis.isAuto()) {
				base = item.basis.resolve(inner_main, content_main);
			}
			else if (!main_length.isAuto()) {
				base = main_length.resolve(inner_main, content_main);
			}
			else {
				base = static_cast<double>(content_main);
			}
			w.base = (std::max)(0.0, base);
			w.hypothetical = clamp_double(w.base, w.minMain, w.maxMain);

			w.crossIsAuto = cross_length.isAuto();
			double cross = cross_length.isAuto()
				? static_cast<double>(content_cross)
				: static_cast<double>(cross_length.resolve(inner_cross, content_cross));
			w.cross = clamp_double((std::max)(0.0, cross), w.minCross, w.maxCross);
			w.align = resolve_align(item, spec.align);
		}

		// ---- 分行 ----
		std::vector<Line> lines;
		if (items.empty()) {
			result.content = LayoutSize{ spec.padding.horizontal(), spec.padding.vertical() };
			return result;
		}

		if (!spec.wrap || inner_main == LayoutSize::Unbounded) {
			lines.push_back(Line{ 0, items.size(), 0.0, 0.0 });
		}
		else {
			std::size_t line_begin = 0;
			double used = 0.0;
			for (std::size_t i = 0; i < work.size(); ++i) {
				WorkItem& w = work[i];
				const double outer = w.hypothetical + w.marginMainStart + w.marginMainEnd;
				if (i > line_begin) {
					const double candidate = used + main_gap + outer;
					if (candidate > static_cast<double>(inner_main)) {
						lines.push_back(Line{ line_begin, i, 0.0, 0.0 });
						line_begin = i;
						used = 0.0;
					}
				}
				if (i > line_begin) used += main_gap;
				used += outer;
			}
			lines.push_back(Line{ line_begin, work.size(), 0.0, 0.0 });
		}

		// ---- 每条行内解析弹性长度 + 交叉轴尺寸 ----
		for (Line& line : lines) {
			resolve_flexible_lengths(work, line.begin, line.end, inner_main, main_gap);

			double line_cross = 0.0;
			for (std::size_t i = line.begin; i < line.end; ++i) {
				const WorkItem& w = work[i];
				const double outer = w.cross + w.marginCrossStart + w.marginCrossEnd;
				if (outer > line_cross) line_cross = outer;
			}
			// 单行且容器交叉尺寸确定时，行高就是容器内交叉尺寸（CSS 行为）。
			if (!spec.wrap && inner_cross != LayoutSize::Unbounded) {
				line_cross = static_cast<double>(inner_cross);
			}
			line.cross = (std::max)(0.0, line_cross);
		}

		// ---- 行在交叉轴上的分配 ----
		double lines_total = 0.0;
		for (std::size_t i = 0; i < lines.size(); ++i) {
			lines_total += lines[i].cross;
			if (i + 1 < lines.size()) lines_total += cross_gap;
		}

		double cross_free = (inner_cross == LayoutSize::Unbounded)
			? 0.0
			: static_cast<double>(inner_cross) - lines_total;
		if (cross_free < 0.0) cross_free = 0.0;

		double line_offset = 0.0;
		double line_extra_gap = 0.0;
		const double line_count = static_cast<double>(lines.size());
		switch (spec.alignContent) {
		case AlignContent::End:
			line_offset = cross_free;
			break;
		case AlignContent::Center:
			line_offset = cross_free * 0.5;
			break;
		case AlignContent::SpaceBetween:
			if (lines.size() > 1) line_extra_gap = cross_free / (line_count - 1.0);
			break;
		case AlignContent::SpaceAround:
			if (!lines.empty()) {
				const double pad = cross_free / line_count;
				line_offset = pad * 0.5;
				line_extra_gap = pad;
			}
			break;
		case AlignContent::SpaceEvenly:
			if (!lines.empty()) {
				const double pad = cross_free / (line_count + 1.0);
				line_offset = pad;
				line_extra_gap = pad;
			}
			break;
		case AlignContent::Start:
		default:
			break;
		}

		double line_cursor = line_offset;
		for (Line& line : lines) {
			// ---- 行内主轴定位 ----
			double used_main = 0.0;
			for (std::size_t i = line.begin; i < line.end; ++i) {
				used_main += work[i].target + work[i].marginMainStart + work[i].marginMainEnd;
			}
			const std::size_t count = line.end - line.begin;
			if (count > 1) used_main += main_gap * static_cast<double>(count - 1);

			const double main_extent = (inner_main == LayoutSize::Unbounded)
				? used_main
				: static_cast<double>(inner_main);
			const double main_free = main_extent - used_main;

			double justify_offset = 0.0;
			double justify_extra_gap = 0.0;
			const double n = static_cast<double>(count);
			if (main_free > 0.0) {
				switch (spec.justify) {
				case Justify::End:
					justify_offset = main_free;
					break;
				case Justify::Center:
					justify_offset = main_free * 0.5;
					break;
				case Justify::SpaceBetween:
					if (count > 1) justify_extra_gap = main_free / (n - 1.0);
					break;
				case Justify::SpaceAround:
					if (count > 0) {
						const double pad = main_free / n;
						justify_offset = pad * 0.5;
						justify_extra_gap = pad;
					}
					break;
				case Justify::SpaceEvenly:
					if (count > 0) {
						const double pad = main_free / (n + 1.0);
						justify_offset = pad;
						justify_extra_gap = pad;
					}
					break;
				case Justify::Start:
				default:
					break;
				}
			}

			double cursor = justify_offset;
			for (std::size_t i = line.begin; i < line.end; ++i) {
				WorkItem& w = work[i];
				cursor += w.marginMainStart;
				w.mainStart = cursor;
				w.mainEnd = cursor + w.target;
				cursor = w.mainEnd + w.marginMainEnd + main_gap + justify_extra_gap;
			}

			// 反向主轴：以主起点边为 0 的坐标镜像回物理坐标。
			if (reverse) {
				for (std::size_t i = line.begin; i < line.end; ++i) {
					WorkItem& w = work[i];
					const double mirrored_start = main_extent - w.mainEnd;
					w.mainEnd = main_extent - w.mainStart;
					w.mainStart = mirrored_start;
				}
			}

			// ---- 行内交叉轴对齐 ----
			for (std::size_t i = line.begin; i < line.end; ++i) {
				WorkItem& w = work[i];
				if (w.align == Align::Stretch && w.crossIsAuto) {
					const double stretched = line.cross - w.marginCrossStart - w.marginCrossEnd;
					w.cross = clamp_double((std::max)(0.0, stretched), w.minCross, w.maxCross);
				}
				const double outer = w.cross + w.marginCrossStart + w.marginCrossEnd;
				switch (w.align) {
				case Align::End:
					w.crossStart = line_cursor + line.cross - w.marginCrossEnd - w.cross;
					break;
				case Align::Center:
					w.crossStart = line_cursor + (line.cross - outer) * 0.5 + w.marginCrossStart;
					break;
				case Align::Start:
				case Align::Stretch:
				case Align::Auto:
				default:
					w.crossStart = line_cursor + w.marginCrossStart;
					break;
				}
			}

			line_cursor += line.cross + cross_gap + line_extra_gap;
		}

		// ---- 输出 ----
		int content_width = spec.padding.horizontal();
		int content_height = spec.padding.vertical();

		for (std::size_t i = 0; i < work.size(); ++i) {
			WorkItem& w = work[i];
			const double left_f = (horizontal ? pad_left + w.mainStart : pad_left + w.crossStart);
			const double right_f = (horizontal ? pad_left + w.mainEnd : pad_left + w.crossStart + w.cross);
			const double top_f = (horizontal ? pad_top + w.crossStart : pad_top + w.mainStart);
			const double bottom_f = (horizontal ? pad_top + w.crossStart + w.cross : pad_top + w.mainEnd);

			LayoutRect rect;
			rect.x = static_cast<int>(std::lround(left_f));
			rect.y = static_cast<int>(std::lround(top_f));
			rect.width = (std::max)(0, static_cast<int>(std::lround(right_f)) - rect.x);
			rect.height = (std::max)(0, static_cast<int>(std::lround(bottom_f)) - rect.y);

			w.spec->result = rect;
			content_width = (std::max)(content_width, rect.right());
			content_height = (std::max)(content_height, rect.bottom());
		}

		result.content = LayoutSize{ content_width, content_height };
		result.lineCount = static_cast<int>(lines.size());
		return result;
	}
}
