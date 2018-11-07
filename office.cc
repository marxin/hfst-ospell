/*

  Copyright 2015 University of Helsinki

  Bug reports for this file should go to:
  Tino Didriksen <mail@tinodidriksen.com>
  Code adapted from https://github.com/TinoDidriksen/trie-tools

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

#include "liboffice.h"
#include <iostream>
#include <vector>
#include <string>

int main(int argc, char **argv) {
    bool verbatim = false;

    std::vector<std::string> args(argv, argv+argc);
	for (auto it=args.begin() ; it != args.end() ; ) {
		if (*it == "--verbatim") {
			verbatim = true;
			it = args.erase(it);
		}
		else {
			++it;
		}
	}

    if (args.size() < 2) {
        throw std::invalid_argument("Must pass a zhfst as argument");
    }

    hfst_ospell::OfficeSpeller speller(args[1].c_str(), verbatim);
    speller.ispell(std::cin, std::cout);
}
