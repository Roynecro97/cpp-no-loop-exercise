#!/usr/bin/env python3.8
# PYTHON_ARGCOMPLETE_OK
"""
Template generator for the exercise.
"""

import argparse
import json
import os
import sys

from Cheetah.Template import Template

try:
    import argcomplete
except ImportError:
    pass

SCRIPTS_DIR = os.path.dirname(os.path.realpath(__file__))

with open(os.path.join(SCRIPTS_DIR, 'stages.json')) as stages_db:
    STAGES = json.load(stages_db)


def main():
    parser = argparse.ArgumentParser(description="C++ stage template generator")
    parser.add_argument('stage', metavar='stage', choices=list(STAGES), help="the stage to generate (ie: stage1)")
    parser.add_argument('-o', '--output', help="the output for the generator (default: ./{stage}.cpp)")

    if 'argcomplete' in sys.modules:
        argcomplete.autocomplete(parser)

    args = parser.parse_args()

    with argparse.FileType('w')(args.output or f'{args.stage}.cpp') as output_file:
        output_file.write(str(Template(file=os.path.join(SCRIPTS_DIR, 'main.cpp.tmpl'), namespaces=STAGES[args.stage])))


if __name__ == '__main__':
    main()
