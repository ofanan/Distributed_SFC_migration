LANG=C
#opp_makemake -f --deep
#mkdir -p out/clang-release
#chmod 777 out out/clang-release
#chmod 777 Makefile
#chmod 777 out/clang-release/*
#chmod 777 Distributed_SFC_migration
#make
clear
out/clang-release/Distributed_SFC_migration -f Common.ini UniformTree.ini -u Cmdenv

