#!/bin/bash
git add *; git commit --amend -am "backup commit"
cd build; make -j 16; ./tlc_test -v | tee out.log; diff out.log ../../tl-test/build/out.log

