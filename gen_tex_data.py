import subprocess

FORMAT = r"""\def\rmsuccesspassed{%s}
\def\rmsuccessfailed{%s}
\def\edfsuccesspassed{%s}
\def\edfsuccessfailed{%s}
\def\rmfailurepassed{%s}
\def\rmfailurefailed{%s}
\def\edffailurepassed{%s}
\def\edffailurefailed{%s}
\def\totalsuccess{%s}
\def\totalfailure{%s}
\def\totalsets{%s}"""

rm_stats = subprocess.run('python3 analyze.py --stats success_data/rm/data_*.pickle', shell=True, capture_output=True)
rm_stats = rm_stats.stdout.decode('utf-8')
rmsuccesstotal, rmsuccessfailed, rmsuccesspassed = (rm_stats.split()[3], rm_stats.split()[5], rm_stats.split()[7])

rm_stats = subprocess.run('python3 analyze.py --stats failure_data/rm/data_*.pickle', shell=True, capture_output=True)
rm_stats = rm_stats.stdout.decode('utf-8')
rmfailuretotal, rmfailurefailed, rmfailurepassed = (rm_stats.split()[3], rm_stats.split()[5], rm_stats.split()[7])

edf_stats = subprocess.run('python3 analyze.py --stats success_data/edf/data_*.pickle', shell=True, capture_output=True).stdout.decode('utf-8')

edfsuccesstotal, edfsuccessfailed, edfsuccesspassed = (edf_stats.split()[3], edf_stats.split()[5], edf_stats.split()[7])

edf_stats = subprocess.run('python3 analyze.py --stats failure_data/edf/data_*.pickle', shell=True, capture_output=True).stdout.decode('utf-8')

edffailuretotal, edffailurefailed, edffailurepassed = (edf_stats.split()[3], edf_stats.split()[5], edf_stats.split()[7])

if (edfsuccesstotal != rmsuccesstotal or edffailuretotal != rmfailuretotal):
    raise RuntimeError("Invalid totals, have you not run part of the task set?")

print(FORMAT % (rmsuccesspassed, rmsuccessfailed, edfsuccesspassed, edfsuccessfailed, rmfailurepassed, rmfailurefailed, edffailurepassed, edffailurefailed, edfsuccesstotal, edffailuretotal, str(int(edffailuretotal) + int(edfsuccesstotal))))

