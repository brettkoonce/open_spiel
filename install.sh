# Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/usr/bin/env bash


# The following should be easy to setup as a submodule:
# https://git-scm.com/docs/git-submodule

set -e  # exit when any command fails
set -x

if [[ "$OSTYPE" == "linux-gnu" ]]; then
  sudo apt-get update
  sudo apt-get install git virtualenv cmake python3 python3-dev python3-pip python3-setuptools python3-wheel
  if [[ "$TRAVIS" ]]; then
    sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python${OS_PYTHON_VERSION} 10
  fi
elif [[ "$OSTYPE" == "darwin"* ]]; then  # Mac OSX
  brew install python3 gcc@7
  curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
  python3 get-pip.py
  pip3 install virtualenv
else
  echo "The OS '$OSTYPE' is not supported (Only Linux and MacOS is). " \
       "Feel free to contribute the install for a new OS."
  exit 1
fi

git clone -b 'v2.2.4' --single-branch --depth 1 https://github.com/pybind/pybind11.git
# TODO: Point to the official  https://github.com/dds-bridge/dds.git
# when pull requests are in
git clone -b 'develop' --single-branch --depth 1 https://github.com/jblespiau/dds.git  open_spiel/games/bridge/double_dummy_solver
git clone -b 'master' --single-branch --depth 1 https://github.com/abseil/abseil-cpp.git open_spiel/abseil-cpp
