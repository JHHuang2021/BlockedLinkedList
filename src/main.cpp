#include <climits>
#include <cstring>

#include "blocked_link_list.hpp"
struct String {
  char ch[65];
  bool operator<(const String &rhs) const { return strcmp(ch, rhs.ch) < 0; }
  bool operator==(const String &rhs) const { return strcmp(ch, rhs.ch) == 0; }
} str;
int value;
BlockedLinkList<String, int, INT_MIN> bll("test");
int main() {
  freopen("test.in", "r", stdin);
  freopen("test.out", "w", stdout);
  int n;
  int cnt = 0;
  std::cin >> n;
  for (int i = 0; i < n; i++) {
    std::string op;
    std::cin >> op;
    if (op != "find")
      std::cin >> str.ch >> value;
    else
      std::cin >> str.ch;
    try {
      if (op == "insert")
        bll.Insert(str, value);
      else if (op == "delete")
        bll.Delete(str, value);
      else if (op == "find") {
        cnt++;
        std::vector<int> ret = bll.Find(str);
        if (!ret.empty()) {
          for (int i : ret) std::cout << i << " ";
          std::cout << std::endl;
        } else
          std::cout << "null" << std::endl;
      }
    } catch (...) {
      // std::cout << "null" << std::endl;
    }
  }

  return 0;
}