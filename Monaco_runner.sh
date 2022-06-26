LANG=C
opp_makemake -f --deep
mkdir -p out/clang-release
chmod 777 out out/clang-release
chmod 777 Makefile
make -i
clear
out/clang-release/Distributed_SFC_migration -f Common.ini Monaco.ini -u Cmdenv

