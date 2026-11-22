#include "./Diff.hpp"
#include <map>
#include <utility>

using namespace std;
using namespace w32oop::ui;

ChildPlan w32oop::ui::plan_children(
	const vector<wstring>& old_keys,
	const vector<type_index>& old_types,
	const vector<wstring>& new_keys,
	const vector<type_index>& new_types)
{
	ChildPlan plan;
	plan.reuse.assign(new_keys.size(), -1);

	const size_t old_count = old_keys.size();
	vector<bool> used(old_count, false);
	vector<bool> replaced(old_count, false);

	map<wstring, size_t> by_key;
	for (size_t i = 0; i < old_count; ++i) {
		if (!old_keys[i].empty() && by_key.find(old_keys[i]) == by_key.end()) {
			by_key.emplace(old_keys[i], i);
		}
	}

	for (size_t i = 0; i < new_keys.size(); ++i) {
		const type_index& new_type = new_types[i];

		if (!new_keys[i].empty()) {
			auto it = by_key.find(new_keys[i]);
			if (it != by_key.end()) {
				const size_t old_index = it->second;
				if (!used[old_index] && old_types[old_index] == new_type) {
					plan.reuse[i] = static_cast<int>(old_index);
					used[old_index] = true;
					continue;
				}
				if (!used[old_index]) {
					used[old_index] = true;
					replaced[old_index] = true;
				}
				continue;
			}
			continue;
		}

		if (i < old_count && !used[i] && old_keys[i].empty() && old_types[i] == new_type) {
			plan.reuse[i] = static_cast<int>(i);
			used[i] = true;
		}
	}

	for (size_t i = 0; i < old_count; ++i) {
		if (!used[i] || replaced[i]) plan.unmount.push_back(static_cast<int>(i));
	}

	return plan;
}
