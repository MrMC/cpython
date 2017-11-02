import sys
from test import regrtest

result = regrtest.main_in_temp_cwd()
sys.exit(result)
