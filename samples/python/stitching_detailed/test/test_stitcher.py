import unittest
import os
import sys

import numpy as np
import cv2 as cv

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__),
                                                '..', '..')))

from stitching_detailed.stitcher import Stitcher
from stitching_detailed.image_to_megapix_scaler import ImageToMegapixScaler
from stitching_detailed.feature_detector import FeatureDetector
from stitching_detailed.feature_matcher import FeatureMatcher
from stitching_detailed.camera_estimator import CameraEstimator
from stitching_detailed.camera_adjuster import CameraAdjuster
from stitching_detailed.camera_wave_corrector import WaveCorrector
# %%

# Eine Klasse erstellen, die von unittest.TestCase erbt
class TestStitcher(unittest.TestCase):
    def setUp(self):
        self.settings = {
            "try_cuda": False,
            "work_megapix": 0.6,
            "features": "orb",
            "matcher": "homography",
            "estimator": "homography",
            "match_conf": None,
            "conf_thresh": 1.0,
            "ba": "ray",
            "ba_refine_mask": "xxxxx",
            "wave_correct": "no",
            "save_graph": None,
            "warp": "spherical",
            "seam_megapix": 0.1,
            "seam": "dp_color",
            "compose_megapix": -1,
            "expos_comp": "gain_blocks",
            "expos_comp_nr_feeds": 1,
            "expos_comp_nr_filtering": 2,
            "expos_comp_block_size": 32,
            "blend": "multiband",
            "blend_strength": 5,
            "output": "result.jpg",
            "timelapse": None,
            "rangewidth": -1
            }

    def tearDown(self):
        try:
            os.remove("result.jpg")
        except OSError:
            pass

    def test_stitcher_aquaduct(self):
        stitcher = Stitcher(["s1.jpg", "s2.jpg"], **self.settings)
        stitcher.stitch()

        max_image_shape_derivation = 3
        np.testing.assert_allclose(stitcher.result.shape[:2],
                                   (700, 1811),
                                   atol=max_image_shape_derivation)

    @unittest.skip("skip boat test (high resuolution ran >5s)")
    def test_stitcher_boat(self):
        stitcher = Stitcher(["boat1.jpg", "boat2.jpg",
                             "boat3.jpg", "boat4.jpg",
                             "boat5.jpg", "boat6.jpg"], **self.settings)
        stitcher.stitch()

        max_image_shape_derivation = 100
        np.testing.assert_allclose(stitcher.result.shape[:2],
                                   (2667, 10751),
                                   atol=max_image_shape_derivation)

    def test_image_to_megapix_scaler(self):
        img1, img2 = cv.imread("s1.jpg"), cv.imread("s2.jpg")
        scaler = ImageToMegapixScaler(0.6)
        self.assertEqual(scaler.get_scale(img1), 0.8294067854101966)
        resized = scaler.resize_to_scale(img1, scaler.get_scale(img1))
        self.assertEqual(resized.shape, (581, 1033, 3))

        resized = scaler.set_scale_and_downscale(img1)
        self.assertEqual(resized.shape, (581, 1033, 3))

        resized = scaler.set_scale_and_downscale(img2)
        self.assertEqual(resized.shape, (581, 1149, 3))
        self.assertIsNot(scaler.scale, scaler.get_scale(img2))

        scaler = ImageToMegapixScaler(2)
        self.assertEqual(scaler.get_scale(img1), 1.5142826857233715)
        self.assertEqual(scaler.get_scale_to_force_downscale(img1), 1.0)

    def test_feature_detector(self):
        img1 = cv.imread("s1.jpg")

        default_number_of_keypoints = 500
        detector = FeatureDetector("orb")
        features = detector.detect_features(img1)
        self.assertEqual(len(features.getKeypoints()),
                         default_number_of_keypoints)

        other_keypoints = 100
        detector = FeatureDetector("orb", nfeatures=other_keypoints)
        features = detector.detect_features(img1)
        self.assertEqual(len(features.getKeypoints()), other_keypoints)

    def test_feature_matcher(self):
        img1, img2 = cv.imread("s1.jpg"), cv.imread("s2.jpg")

        detector = FeatureDetector("orb")
        features = [detector.detect_features(img1),
                    detector.detect_features(img2)]

        matcher = FeatureMatcher()
        pairwise_matches = matcher.match_features(features)
        self.assertEqual(len(pairwise_matches), len(features)**2)
        self.assertGreater(pairwise_matches[1].confidence, 2)

    def test_equality_of_focals(self):
        img1, img2 = cv.imread("s1.jpg"), cv.imread("s2.jpg")
        detector = FeatureDetector("orb")
        features = [detector.detect_features(img1),
                    detector.detect_features(img2)]
        matcher = FeatureMatcher()
        pairwise_matches = matcher.match_features(features)

        estimator = CameraEstimator()
        adjuster = CameraAdjuster()
        corrector = WaveCorrector('horiz')
        cameras_estimated = estimator.estimate(features, pairwise_matches)
        cameras_adjusted = adjuster.adjust(features, pairwise_matches,
                                           cameras_estimated)
        cameras_wave_corrected = corrector.correct(cameras_adjusted)

        self.assertEqual(cameras_adjusted[0].focal,
                          cameras_wave_corrected[0].focal)

def starttest():
    unittest.main()


if __name__ == "__main__":
    starttest()
