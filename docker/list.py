#!/usr/bin/env python3.8
# PYTHON_ARGCOMPLETE_OK
"""
Stage list for the exercise.
"""

import argparse
import json
import os
import sys
import textwrap

try:
    import argcomplete
except ImportError:
    pass

SCRIPTS_DIR = os.path.dirname(os.path.realpath(__file__))

with open(os.path.join(SCRIPTS_DIR, 'stages.json')) as stages_db:
    STAGES = json.load(stages_db)


def main():
    parser = argparse.ArgumentParser(description="list the available stages")

    if 'argcomplete' in sys.modules:
        argcomplete.autocomplete(parser)

    parser.parse_args()

    for stage, info in STAGES.items():
        stage_title = f'{stage} - '
        stage_desc = '\n'.join(textwrap.wrap(info['desc'], subsequent_indent=' ' * len(stage_title)))
        print(f'{stage_title}{stage_desc}')


if __name__ == '__main__':
    main()
