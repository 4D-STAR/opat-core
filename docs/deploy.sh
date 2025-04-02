#/bin/bash

# this scirpt copies the documentation files to the local directory

cp -r ../opatIO-cpp/docs/html ./cpp
cp -r ../opatIO-py/docs/build/html ./py

touch .nojekyll
touch cpp/.nojekyll
touch py/.nojekyll