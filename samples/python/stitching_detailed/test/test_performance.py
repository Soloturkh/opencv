import unittest
import os
import sys
import time
import tracemalloc

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__),
                                                '..', '..')))

from stitching_detailed.stitcher import Stitcher
from stitching_detailed_old import main
# %%


class TestStitcher(unittest.TestCase):

    def test_performance(self):

        print("Run new Stitcher class:")

        start = time.time()
        tracemalloc.start()

        stitcher = Stitcher(compose_megapix=3)
        stitcher.stitch(["boat5.jpg", "boat2.jpg",
                         "boat3.jpg", "boat4.jpg",
                         "boat1.jpg", "boat6.jpg"])
        stitcher.collect_garbage()

        current_memory, peak_memory = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        end = time.time()
        time_needed = end - start

        print(f"Peak was {peak_memory / 10**6} MB")
        print(f"Time was {time_needed} s")

        print("Run original stitching_detailed.py:")

        start = time.time()
        tracemalloc.start()

        main()

        _, peak_memory_old = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        end = time.time()
        time_needed_old = end - start

        print(f"Peak was {peak_memory_old / 10**6} MB")
        print(f"Time was {time_needed_old} s")

        self.assertLessEqual(current_memory / 10**6, 0.2)
        self.assertLessEqual(peak_memory / 10**6, peak_memory_old / 10**6)
        uncertainty_based_on_run = 0.25
        self.assertLessEqual(time_needed - uncertainty_based_on_run,
                             time_needed_old)


def starttest():
    unittest.main()


if __name__ == "__main__":
    starttest()
