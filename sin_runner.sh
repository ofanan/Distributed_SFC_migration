
LANG=C
opp_makemake -f --deep
mkdir -p out/clang-release
chmod 777 out out/clang-release
chmod 777 Makefile
make -i
out/clang-release/Distributed_SFC_migration -f MonacoC.ini -u Cmdenv

