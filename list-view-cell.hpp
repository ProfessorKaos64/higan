#if defined(Hiro_ListView)

namespace hiro {

struct pListViewCell : pObject {
  Declare(ListViewCell, Object)

  auto setAlignment(Alignment alignment) -> void;
  auto setBackgroundColor(Color color) -> void;
  auto setCheckable(bool checkable) -> void;
  auto setChecked(bool checked) -> void;
  auto setFont(const string& font) -> void;
  auto setForegroundColor(Color color) -> void;
  auto setImage(const Image& image) -> void;
  auto setText(const string& text) -> void;

  auto _parent() -> maybe<pListViewItem&>;
  auto _setState() -> void;
};

}

#endif
