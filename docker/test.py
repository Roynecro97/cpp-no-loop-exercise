#!/usr/bin/env python3.8
# PYTHON_ARGCOMPLETE_OK
"""
Solution tester for the exercise.
"""

import argparse
import itertools
import json
import os
import subprocess
import sys
import textwrap

try:
    import argcomplete
except ImportError:
    pass

SCRIPTS_DIR = os.path.dirname(os.path.realpath(__file__))

with open(os.path.join(SCRIPTS_DIR, 'stages.json')) as stages_db:
    STAGES = json.load(stages_db)


def show_output(title, raw):
    print(f'{title}:')
    print(textwrap.indent(raw, '> '))


def run_test(stage_binary, name, input, output):
    if isinstance(input, list):
        input = '\n'.join(input)

    print(f'{name}: ', end='')
    sys.stdout.flush()

    run_result = subprocess.run([stage_binary], input=input + '\n', text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
    run_output =  run_result.stdout.rstrip()
    if run_result.returncode != 0 or run_output != output:
        print('Failed')
        show_output('Input', input)
        if run_result.stderr:
            show_output('Standard Error', run_result.stderr.rstrip())
        show_output('Standard Output', run_output)
        show_output('Expected', output)
        return False
    print('OK')
    return True


def main():
    parser = argparse.ArgumentParser(description="run the tests on the given stage. note: this does not compile the stage")
    parser.add_argument('stage', metavar='stage', choices=list(STAGES), help="the stage to test (ie: stage1)")


    if 'argcomplete' in sys.modules:
        argcomplete.autocomplete(parser)

    args = parser.parse_args()

    return all(run_test(f'./{args.stage}', **test) for test in STAGES[args.stage]['test_cases'])


if __name__ == '__main__':
    sys.exit(0 if main() else 1)
