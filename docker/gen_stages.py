#!/usr/bin/env python3
"""
Generates the stages json.

Run from the "docker/" directory.
"""

import json
import random
import textwrap

from typing import Any, Callable, Dict, Iterable, Text, TypeVar

T = TypeVar('T', int, float)

def generate_series(gen_func: Callable[[int, T], T], count: int = 60, init: T = 0) -> Iterable[T]:
    """
    Generate a series of ``count`` values using ``gen_func``.

    ``gen_func`` is called with the index of the current element and the
    last integer that it returned (using ``init`` for the first call) and
    should return the value that should be in the given index (called in
    sequence).
    """
    value = init
    for i in range(count):
        value = gen_func(i, value)
        yield value


def join_any(values: Iterable[Any]) -> Text:
    """
    Join any iterable with a space.
    """
    return ' '.join(str(val) for val in values)


Solution = Callable[[Text], Text]


def stage1_solution(text: Text) -> Text:
    """
    Solve stage1.
    """
    return '\n'.join(text.split())


def stage2_solution(text: Text) -> Text:
    """
    Solve stage2.
    """
    values = [int(num) for num in text.split()]
    return f'{sum(values) // len(values)}'


def stage3_solution(text: Text) -> Text:
    """
    Solve stage3.
    """
    return ', '.join(text.split())


def stage4_solution(text: Text) -> Text:
    """
    Solve stage4.
    """
    values = [int(num) for num in text.split()]
    return 'OK' if all(a * 2 <= b for a, b in zip(values, values[1:])) else 'BAD'


def stage5_solution(text: Text) -> Text:
    """
    Solve stage5.
    """
    if not text:
        return ''
    return max(text.split(), key=lambda word: sum(word.lower().count(v) for v in 'aeiou'))


def test_case(name: Text, input_str: Text, solution: Solution) -> Dict[Text, Text]:
    """
    Generate a test-case dict using the ``solution`` function.
    """
    return {
        'name': name,
        'input': input_str,
        'output': solution(input_str),
    }


STAGES = {
    'stage1': {
        'desc': 'Print all elements in the container, each in a new line',
        'includes': ['list'],
        'container_type': 'list<int>',
        'container_name': 'ints',
        'element_type': 'int',
        'examples': 1,
        'test_cases': [
            test_case('4 numbers', '10 20 30 40', stage1_solution),
            test_case('1 number', '510', stage1_solution),
            test_case('0 numbers', '', stage1_solution),
        ],
    },
    'stage2': {
        'desc': 'Calculate the average of the values in the vector',
        'includes': ['vector'],
        'container_type': 'vector<int>',
        'container_name': 'ints',
        'element_type': 'int',
        'examples': 1,
        'test_cases': [
            test_case('4 numbers', '10 20 30 40', stage2_solution),
            test_case('5 numbers', '1 2 3 4 5', stage2_solution),
            test_case('2 numbers', '5 8', stage2_solution),
            test_case('100 numbers', join_any(random.randint(-10, 200) for _ in range(100)), stage2_solution),
            test_case('1 number', '510', stage2_solution),
            test_case('negative average', '-10 2', stage2_solution),
        ],
    },
    'stage3': {
        'desc': "Print all of the words in the container, separated by comma (Python: ', '.join(words))",
        'includes': ['vector', 'string'],
        'container_type': 'vector<string>',
        'container_name': 'words',
        'element_type': 'string',
        'examples': 1,
        'test_cases': [
            test_case('fruits', 'apple banana pineapple mango', stage3_solution),
            test_case('lorem ipsum',
                      'Esse deserunt tempor ipsum ipsum anim excepteur esse excepteur nulla est Lorem in amet voluptate',
                      stage3_solution),
            test_case('nothing', '', stage3_solution),
            test_case('1 word', 'C++', stage3_solution),
        ],
    },
    'stage4': {
        'desc': textwrap.dedent('''
            Check if every value in the vector is at least twice as big as the previous value (ints[i] * 2 <= ints[i + 1]).
            If they are, print "OK". If not print "BAD".
            ''').strip(),
        'includes': ['vector', 'cstdint'],
        'container_type': 'vector<uint64_t>',
        'container_name': 'ints',
        'element_type': 'uint64_t',
        'examples': 2,
        'test_cases': [
            test_case('simple', '1 2 5 10', stage4_solution),
            test_case('bad', '1 2 3 4', stage4_solution),
            test_case('not rising', '10 20 5 46', stage4_solution),
            test_case('many numbers',
                      join_any(generate_series(lambda idx, prev: prev * 2 + random.choice([0] * 5 + list(range(1, 11))))),
                      stage4_solution),
            test_case('single', '36', stage4_solution),
            test_case('empty', '', stage4_solution),
            test_case('bad first value', '5 8 16 32', stage4_solution),
            test_case('bad last value', '10 20 80 46', stage4_solution),
        ],
    },
    'stage5': {
        'desc': 'Print the word that contains the highest amount of vowels ("aeiou").',
        'includes': ['vector', 'string'],
        'container_type': 'vector<string>',
        'container_name': 'words',
        'element_type': 'string',
        'examples': 2,
        'test_cases': [
            test_case('simple', 'hello world', stage5_solution),
            test_case('caps', 'HELLO WORLD', stage5_solution),
            test_case('mixed', 'DolOre lAborE doloR nISi lAbORum ut cUpiDatAt teMpoR alIQuIp FuGiAt', stage5_solution),
            test_case('some words',
                      'Ut incididunt adipisicing eu et deserunt consectetur enim ullamco magna cillum tempor enim fugiat incididunt',
                      stage5_solution),
            test_case('no vowels', 'Mnd smd t lbrm st psm', stage5_solution),
            test_case('1 word', 'nothing', stage5_solution),
            test_case('nothing', '', stage5_solution),
        ],
    },
}


def main():
    with open('stages.json', 'w') as jf:
        json.dump(STAGES, jf, indent=4)
        print(file=jf)  # Add a final newline


if __name__ == '__main__':
    main()
