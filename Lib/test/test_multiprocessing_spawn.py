import os
import unittest

if not os.allows_subprocesses:
    raise unittest.SkipTest('Test requires support for subprocesses.')

import test._test_multiprocessing

test._test_multiprocessing.install_tests_in_module_dict(globals(), 'spawn')


if __name__ == '__main__':
    unittest.main()
