from __future__ import print_function

from datetime import datetime
import platform
from test import regrtest

regrtest.start = datetime.now()
print("Testing on %s" % platform.machine())
print("START:", regrtest.start)
regrtest.main_in_temp_cwd()
regrtest.end = datetime.now()
print("END:", regrtest.end)
print("Duration:", regrtest.end - regrtest.start)

