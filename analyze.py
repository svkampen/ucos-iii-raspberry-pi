"""
Data analysis script. Takes the pickled output from serial_controller.py and turns it into plots.

Usage:
    analyze.py [options] <filename>...

Options:
    --ylog             Make the y-axis logarithmic.
    --total            Plot the task sets as a whole, not separated into successful/failed.
    -p, --percentage   Plot values are percentages, not raw numbers.
    -b, --bar          Plot a bar plot instead of a line plot.
    -r, --round=<val>  Round processor utilization to val% [default: 5]
    -o <output>        Write the output figure to <output>.
    -s, --stats        Don't plot anything, just print statistics on the dataset.
    -h, --help         Display this help text.
"""
import operator
import pickle
import docopt
import itertools
import numpy as np
from matplotlib import pyplot as plt, rcParams
from collections import defaultdict, Counter
from typing import Dict, List
from task_generate import TaskSet, Task

ROUND_TO = None

rcParams['figure.autolayout'] = True

def to_rounded_utilization(task_set: TaskSet, round_to_percent: int = None) -> float:
    round_to_percent = round_to_percent or ROUND_TO
    if round_to_percent < 1: raise ValueError(f"Invalid rounding percentage: {round_to_percent}")
    return round(task_set.get_utilization()*100 / round_to_percent)*round_to_percent

def stats(data):
    if isinstance(data, dict):
        failed_num = len(data['fail'])
        succ_num   = len(data['success'])
        total_num = len(data['success']) + len(data['fail'])
        print(f"Total task sets: {total_num}")
        mean = np.mean([len(t.tasks) for t in data['fail']])
        max = np.max([len(t.tasks) for t in data['fail']])
        min = np.min([len(t.tasks) for t in data['fail']])
        print(f"Failed: {failed_num} Succeeded: {succ_num} Mean #tasks: {mean} Max: {max} Min: {min}")
    if isinstance(data, list):
        print(f"Total task sets: {len(data)}")

def main(args):
    global ROUND_TO
    ROUND_TO = int(args['--round'])

    data: Dict[str, List[TaskSet]] = {'success': [],
                                      'fail': []}

    for filename in args['<filename>']:
        with open(filename, 'rb') as f:
            fdata = pickle.load(f)
            if isinstance(fdata, dict):
                data['success'] += fdata['success']
                data['fail'] += fdata['fail']
            else:
                data['fail'] += fdata

    if (args['--stats']):
        return stats(data)

    success_ctr = Counter(map(to_rounded_utilization, data['success']))
    failure_ctr = Counter(map(to_rounded_utilization, data['fail']))

    min_util = min(itertools.chain(failure_ctr, success_ctr))
    max_util = max(itertools.chain(failure_ctr, success_ctr))

    total_map = {i:success_ctr[i] + failure_ctr[i] for i in range(min_util, max_util + ROUND_TO, ROUND_TO)}

    print(total_map)

    for i in range(min_util, max_util + ROUND_TO, ROUND_TO):
        if args['--bar']:
            success_ctr[i] = 0 if not success_ctr[i] else success_ctr[i]
            failure_ctr[i] = 0 if not failure_ctr[i] else failure_ctr[i]
        if (success_ctr[i] == 0 and failure_ctr[i] == 0):
            continue
        else:
            if not args['--percentage']:
                continue
            success_ctr[i] /= total_map[i]
            failure_ctr[i] /= total_map[i]
            success_ctr[i] *= 100
            failure_ctr[i] *= 100

    failure_xs, failure_ys = zip(*sorted(failure_ctr.items(), key=operator.itemgetter(0)))
    success_xs, success_ys = zip(*sorted(success_ctr.items(), key=operator.itemgetter(0)))

    print(success_ctr, failure_ctr)

    width = ROUND_TO * 0.80

    succ_color = 'cornflowerblue'
    fail_color = succ_color if args['--total'] else 'tab:pink'

    if args['--bar']:
        p1 = plt.bar(success_xs, success_ys, width, color=succ_color)
        # bottom adjustment because the generated .eps shows a seam between the fail/success
        # parts which is very visible in --total mode. And the -0.5 doesn't make a large difference.
        p2 = plt.bar(failure_xs, failure_ys, width, bottom=[y - (0.5 if args['--total'] else 0) for y in success_ys], color=fail_color)
        if args['--total']:
            plt.ylim(0)
    else:
        p1 = plt.plot(success_xs, success_ys)
        p2 = plt.plot(failure_xs, failure_ys)
    if not args['--total']:
        plt.legend((p1[0], p2[0]), ('Successful task sets', 'Failing task sets'))
    if (args['--bar']):
        # plt.xticks(ticks=failure_xs, labels=[f'{i}-{i+ROUND_TO}' for i in failure_xs], rotation=45)
        # plt.xticks(ticks=failure_xs, labels=[f'{i}' for i in failure_xs], rotation=45)
        pass
    if args['--percentage']:
        plt.yticks(ticks=range(0,110,10))
    plt.xlabel('Processor utilization (%)')

    if args['--ylog']:
        axes = plt.gca()
        axes.set_yscale('log')
        plt.ylim(0.1)

    ylabel = 'Task sets' if not args['--percentage'] else 'Task sets (%)'

    plt.ylabel(ylabel)
    if (args['-o']):
        plt.savefig(args['-o'])
    else:
        plt.show()


main(docopt.docopt(__doc__))
