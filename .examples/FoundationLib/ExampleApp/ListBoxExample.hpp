#pragma once
#include "../../../w32use.hpp"

// ListBox example.
//
// Two list boxes share the same window:
//   * left  - a plain single-selection list box (LBS_NOTIFY comes from
//             ListBox::STYLE, together with WS_VSCROLL and WS_BORDER)
//   * right - a sorted, multiple-selection list box created with
//             LBS_SORT | LBS_MULTIPLESEL | LBS_EXTENDEDSEL
//
// Things worth noticing:
//   * addItem() returns the index the string ended up at, which is not the
//     insertion order once LBS_SORT is in the style
//   * setItemText() deletes and re-inserts the string, but it keeps both the
//     item data and the current selection
//   * getSelectedItems() / setSelectionState() only make sense for a list box
//     that owns LBS_MULTIPLESEL; multiple() reports whether it does
//   * LBN_SELCHANGE fires for every selection change and LBN_DBLCLK for a
//     double click, so the labels below can always show the live state
class ListBoxExample : public ChildControlWindow {
public:
	ListBoxExample(HWND parent) : ChildControlWindow(parent, L"ListBox example", 1, 1) {}

protected:
	LinearLayout coreLayout;
	ConstraintLayout layout;

	Static lblSingleHeader;
	ListBox lstSingle;
	Static lblSingleInfo;

	Static lblMultiHeader;
	ListBox lstMulti;
	Static lblMultiInfo;

	Button btnAdd, btnInsert, btnDelete, btnClearAll;
	Button btnRename, btnItemData, btnSelectFirst, btnSelectLast;

	Button btnSelectEven, btnSelectAll, btnClearMulti, btnRefresh;

	Static lblStatus;

	int nextItem = 4;

	void setup_event_handlers() override {
		setLayoutManager(&coreLayout);
		WINDOW_add_handler(WM_SIZE, [this](EventData&) {
			if (lblStatus.created()) updateInfo();
		});
	}

	void onCreated() override {
		createHeaders();
		createLists();
		createInfoLabels();
		createButtons();
		createStatusLabel();

		coreLayout.define(layout).size(460, 405).gravityCenter();

		populateLists();
		wireEventHandlers();

		updateInfo();
		setStatus(L"Ready. Double-click an item to rename it.");
	}

	// ---------------------------------------------------------------- layout

	void createHeaders() {
		lblSingleHeader.set_parent(this);
		lblSingleHeader.create(L"Single-selection list", 1, 1);
		layout.define(lblSingleHeader).width(220).height(18)
			.constraintStart_toStartOf(LayoutRef{})
			.constraintTop_toTopOf(LayoutRef{});

		lblMultiHeader.set_parent(this);
		lblMultiHeader.create(L"Multiple-selection list", 1, 1);
		layout.define(lblMultiHeader).width(220).height(18)
			.constraintStart_toEndOf(lblSingleHeader).marginStart(20)
			.constraintTop_toTopOf(lblSingleHeader);
	}

	void createLists() {
		lstSingle.set_parent(this);
		lstSingle.create(L"", 1, 1);
		layout.define(lstSingle).width(220).height(185)
			.constraintStart_toStartOf(lblSingleHeader)
			.constraintTop_toBottomOf(lblSingleHeader).marginTop(4);

		// LBS_SORT and LBS_MULTIPLESEL are creation-time styles, so they have
		// to be passed to create() instead of being added afterwards.
		lstMulti.set_parent(this);
		lstMulti.create(L"", 1, 1, 0, 0,
			ListBox::STYLE | LBS_SORT | LBS_MULTIPLESEL | LBS_EXTENDEDSEL);
		layout.define(lstMulti).width(220).height(185)
			.constraintStart_toStartOf(lblMultiHeader)
			.constraintTop_toTopOf(lstSingle);
	}

	void createInfoLabels() {
		lblSingleInfo.set_parent(this);
		lblSingleInfo.create(L"", 1, 1);
		layout.define(lblSingleInfo).width(220).height(64)
			.constraintStart_toStartOf(lstSingle)
			.constraintTop_toBottomOf(lstSingle).marginTop(8);

		lblMultiInfo.set_parent(this);
		lblMultiInfo.create(L"", 1, 1);
		layout.define(lblMultiInfo).width(220).height(64)
			.constraintStart_toStartOf(lstMulti)
			.constraintTop_toTopOf(lblSingleInfo);
	}

	void createButtons() {
		// Row 1: add / insert / delete / clear the single-selection list.
		btnAdd.set_parent(this);
		btnAdd.create(L"Add item", 1, 1);
		layout.define(btnAdd).width(108).height(26)
			.constraintStart_toStartOf(LayoutRef{})
			.constraintTop_toBottomOf(lblSingleInfo).marginTop(8);

		btnInsert.set_parent(this);
		btnInsert.create(L"Insert at 0", 1, 1);
		layout.define(btnInsert).width(108).height(26)
			.constraintStart_toEndOf(btnAdd).marginStart(8)
			.constraintTop_toTopOf(btnAdd);

		btnDelete.set_parent(this);
		btnDelete.create(L"Delete item", 1, 1);
		layout.define(btnDelete).width(108).height(26)
			.constraintStart_toEndOf(btnInsert).marginStart(8)
			.constraintTop_toTopOf(btnAdd);

		btnClearAll.set_parent(this);
		btnClearAll.create(L"Clear all", 1, 1);
		layout.define(btnClearAll).width(108).height(26)
			.constraintStart_toEndOf(btnDelete).marginStart(8)
			.constraintTop_toTopOf(btnAdd);

		// Row 2: edit the current item and move the selection around.
		btnRename.set_parent(this);
		btnRename.create(L"Rename...", 1, 1);
		layout.define(btnRename).width(108).height(26)
			.constraintStart_toStartOf(btnAdd)
			.constraintTop_toBottomOf(btnAdd).marginTop(6);

		btnItemData.set_parent(this);
		btnItemData.create(L"Item data", 1, 1);
		layout.define(btnItemData).width(108).height(26)
			.constraintStart_toEndOf(btnRename).marginStart(8)
			.constraintTop_toTopOf(btnRename);

		btnSelectFirst.set_parent(this);
		btnSelectFirst.create(L"Select first", 1, 1);
		layout.define(btnSelectFirst).width(108).height(26)
			.constraintStart_toEndOf(btnItemData).marginStart(8)
			.constraintTop_toTopOf(btnRename);

		btnSelectLast.set_parent(this);
		btnSelectLast.create(L"Select last", 1, 1);
		layout.define(btnSelectLast).width(108).height(26)
			.constraintStart_toEndOf(btnSelectFirst).marginStart(8)
			.constraintTop_toTopOf(btnRename);

		// Row 3: drive the multiple-selection list.
		btnSelectEven.set_parent(this);
		btnSelectEven.create(L"Select even", 1, 1);
		layout.define(btnSelectEven).width(108).height(26)
			.constraintStart_toStartOf(btnRename)
			.constraintTop_toBottomOf(btnRename).marginTop(6);

		btnSelectAll.set_parent(this);
		btnSelectAll.create(L"Select all", 1, 1);
		layout.define(btnSelectAll).width(108).height(26)
			.constraintStart_toEndOf(btnSelectEven).marginStart(8)
			.constraintTop_toTopOf(btnSelectEven);

		btnClearMulti.set_parent(this);
		btnClearMulti.create(L"Clear multi", 1, 1);
		layout.define(btnClearMulti).width(108).height(26)
			.constraintStart_toEndOf(btnSelectAll).marginStart(8)
			.constraintTop_toTopOf(btnSelectEven);

		btnRefresh.set_parent(this);
		btnRefresh.create(L"Refresh info", 1, 1);
		layout.define(btnRefresh).width(108).height(26)
			.constraintStart_toEndOf(btnClearMulti).marginStart(8)
			.constraintTop_toTopOf(btnSelectEven);
	}

	void createStatusLabel() {
		lblStatus.set_parent(this);
		lblStatus.create(L"", 1, 1);
		layout.define(lblStatus).width(456).height(20)
			.constraintStart_toStartOf(btnAdd)
			.constraintTop_toBottomOf(btnSelectEven).marginTop(8);
	}

	// ---------------------------------------------------------------- data

	void populateLists() {
		const wchar_t* fruits[] = { L"Apple", L"Banana", L"Cherry" };
		int data = 1;
		for (const wchar_t* fruit : fruits) {
			// The returned index is where the string was actually placed.
			const int index = lstSingle.addItem(fruit);
			lstSingle.setItemData(index, data++);
		}
		// Start with one item selected so the info label is not empty.
		lstSingle.setCurrentSelection(1);

		// LBS_SORT inserts every string at its sorted position, so the list
		// ends up as Blue, Cyan, Green, Purple, Red, Yellow.
		const wchar_t* colors[] = { L"Red", L"Green", L"Blue", L"Yellow", L"Purple", L"Cyan" };
		for (const wchar_t* color : colors) lstMulti.addItem(color);
		for (int i = 0; i < lstMulti.getCount(); ++i) lstMulti.setItemData(i, 100 + i);

		// LBS_EXTENDEDSEL lets the user click more than one entry; the same
		// state can be produced programmatically with setSelectionState().
		lstMulti.setSelectionState(true, 0);
		lstMulti.setSelectionState(true, 2);
	}

	// ---------------------------------------------------------------- events

	void wireEventHandlers() {
		// LBN_SELCHANGE and LBN_SELCANCEL both end up here.
		lstSingle.onChange([this](EventData&) {
			updateInfo();
		});
		lstMulti.onChange([this](EventData&) {
			updateInfo();
		});

		// LBN_DBLCLK: rename the double-clicked item in place.
		lstSingle.onDblclick([this](EventData&) {
			renameCurrentItem();
		});

		btnAdd.onClick([this](EventData&) {
			const std::wstring itemText = L"Item " + std::to_wstring(nextItem);
			const int index = lstSingle.addItem(itemText);
			lstSingle.setItemData(index, nextItem);
			++nextItem;
			lstSingle.setCurrentSelection(index);
			updateInfo();
			setStatus(L"Added \"" + itemText + L"\" at index " + std::to_wstring(index));
		});

		btnInsert.onClick([this](EventData&) {
			const std::wstring itemText = L"Item " + std::to_wstring(nextItem);
			const int index = lstSingle.insertItem(0, itemText);
			lstSingle.setItemData(index, nextItem);
			++nextItem;
			lstSingle.setCurrentSelection(index);
			updateInfo();
			setStatus(L"Inserted \"" + itemText + L"\" at index 0");
		});

		btnDelete.onClick([this](EventData&) {
			const int selection = lstSingle.getCurrentSelection();
			if (selection < 0 || selection >= lstSingle.getCount()) {
				setStatus(L"Delete: no item is selected");
				return;
			}
			const std::wstring removed = lstSingle.getItemText(selection);
			lstSingle.deleteItem(selection);
			// Keep a sensible selection: the next item, or the new last one.
			if (lstSingle.getCount() == 0) {
				lstSingle.clearSelection();
			}
			else {
				const int next = selection < lstSingle.getCount()
					? selection : lstSingle.getCount() - 1;
				lstSingle.setCurrentSelection(next);
			}
			updateInfo();
			setStatus(L"Deleted \"" + removed + L"\"");
		});

		btnClearAll.onClick([this](EventData&) {
			// resetContent() removes every string together with its item data.
			lstSingle.resetContent();
			lstSingle.clearSelection();
			updateInfo();
			setStatus(L"Cleared every item of the single-selection list");
		});

		btnRename.onClick([this](EventData&) {
			renameCurrentItem();
		});

		btnItemData.onClick([this](EventData&) {
			const int selection = lstSingle.getCurrentSelection();
			if (selection < 0 || selection >= lstSingle.getCount()) {
				setStatus(L"Item data: no item is selected");
				return;
			}
			const long long previous =
				static_cast<long long>(lstSingle.getItemData(selection));
			const long long updated = previous + 100;
			lstSingle.setItemData(selection, static_cast<LPARAM>(updated));
			MessageBoxW(hwnd,
				(L"Item " + describeItem(lstSingle, selection)
					+ L"\r\n\r\nItem data: " + std::to_wstring(previous)
					+ L" -> " + std::to_wstring(updated)).c_str(),
				L"Item data", MB_ICONINFORMATION);
			updateInfo();
		});

		btnSelectFirst.onClick([this](EventData&) {
			if (lstSingle.getCount() == 0) {
				setStatus(L"Select first: the list is empty");
				return;
			}
			lstSingle.setCurrentSelection(0);
			updateInfo();
			setStatus(L"Selected the first item");
		});

		btnSelectLast.onClick([this](EventData&) {
			if (lstSingle.getCount() == 0) {
				setStatus(L"Select last: the list is empty");
				return;
			}
			const int last = lstSingle.getCount() - 1;
			lstSingle.setCurrentSelection(last);
			// Selecting an item also scrolls it into view.
			updateInfo();
			setStatus(L"Selected the last item");
		});

		btnSelectEven.onClick([this](EventData&) {
			for (int i = 0; i < lstMulti.getCount(); i += 2) {
				lstMulti.setSelectionState(true, i);
			}
			updateInfo();
			setStatus(L"Selected every other item of the multiple-selection list");
		});

		btnSelectAll.onClick([this](EventData&) {
			for (int i = 0; i < lstMulti.getCount(); ++i) {
				lstMulti.setSelectionState(true, i);
			}
			updateInfo();
			setStatus(L"Selected every item of the multiple-selection list");
		});

		btnClearMulti.onClick([this](EventData&) {
			lstMulti.clearSelection();
			updateInfo();
			setStatus(L"Cleared the selection of the multiple-selection list");
		});

		btnRefresh.onClick([this](EventData&) {
			updateInfo();
			setStatus(L"Info refreshed. The lists have "
				+ std::to_wstring(lstSingle.getCount()) + L" and "
				+ std::to_wstring(lstMulti.getCount()) + L" items.");
		});
	}

	// ---------------------------------------------------------------- helpers

	void renameCurrentItem() {
		const int selection = lstSingle.getCurrentSelection();
		if (selection < 0 || selection >= lstSingle.getCount()) {
			setStatus(L"Rename: no item is selected");
			return;
		}
		InputDialog dialog(L"Rename item", 360, 145, hwnd);
		auto newText = dialog.getInput<std::wstring>(
			L"New text for the selected item:", lstSingle.getItemText(selection));
		if (!newText.has_value()) {
			setStatus(L"Rename cancelled");
			return;
		}
		lstSingle.setItemText(*newText, selection);
		updateInfo();
		setStatus(L"Renamed item " + std::to_wstring(selection) + L" to \"" + *newText + L"\"");
	}

	static std::wstring describeItem(const ListBox& list, int index) {
		return std::to_wstring(index) + L" (\"" + list.getItemText(index) + L"\")";
	}

	void updateInfo() {
		const int singleCount = lstSingle.getCount();
		const int current = lstSingle.getCurrentSelection();

		std::wstring singleInfo = L"Current: ";
		if (current >= 0 && current < singleCount) singleInfo += describeItem(lstSingle, current);
		else singleInfo += L"none";
		singleInfo += L"\r\nItems: " + std::to_wstring(singleCount);
		singleInfo += L"\r\nItem data: ";
		if (current >= 0 && current < singleCount) {
			singleInfo += std::to_wstring(
				static_cast<long long>(lstSingle.getItemData(current)));
		}
		else {
			singleInfo += L"-";
		}
		lblSingleInfo.text(singleInfo);

		const int multiCount = lstMulti.getCount();
		const std::vector<int> selected = lstMulti.getSelectedItems();

		std::wstring multiInfo = L"Selected: ";
		if (selected.empty()) {
			multiInfo += L"none";
		}
		else {
			for (std::size_t i = 0; i < selected.size(); ++i) {
				if (i != 0) multiInfo += L", ";
				multiInfo += std::to_wstring(selected[i]);
			}
			multiInfo += L" (" + std::to_wstring(selected.size())
				+ L" of " + std::to_wstring(multiCount) + L")";
		}
		multiInfo += L"\r\nItems: " + std::to_wstring(multiCount);
		multiInfo += L"\r\nTop index: " + std::to_wstring(lstMulti.getTopIndex());
		lblMultiInfo.text(multiInfo);
	}

	void setStatus(const std::wstring& message) {
		lblStatus.text(L"Last action: " + message);
	}
};

