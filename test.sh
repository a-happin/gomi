#!/bin/sh

f() {
  clang++ -std=c++2b -Wall -Wextra --pedantic-errors -I./include -I./third-party/iutest/include -O2 -pipe -o /tmp/a.out "$i" || return
  /tmp/a.out || return
  g++ -std=c++2b -Wall -Wextra --pedantic-errors -I./include -I./third-party/iutest/include -O2 -pipe -o /tmp/a.out "$i" || return
  /tmp/a.out || return
}

if [[ $# -eq 0 ]]
then
  set -- $(fd --hidden --exclude .git '\.cpp$' test)
fi

for i in "$@"
do
  printf 'Testing %s ...\n' "$i"
  f "$i" || exit $?
done
