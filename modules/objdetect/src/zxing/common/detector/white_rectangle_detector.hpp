// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Tencent is pleased to support the open source community by making WeChat QRCode available.
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Modified from ZXing. Copyright ZXing authors.
// Licensed under the Apache License, Version 2.0 (the "License").

#ifndef __ZXING_COMMON_DETECTOR_WHITER_ECTANGLE_DETECTOR_HPP__
#define __ZXING_COMMON_DETECTOR_WHITER_ECTANGLE_DETECTOR_HPP__

/*
 *  WhiteRectangleDetector.hpp
 *
 *
 *  Created by Luiz Silva on 09/02/2010.
 *  Copyright 2010  authors All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vector>
#include "../../reader_exception.hpp"
#include "../../result_point.hpp"
#include "../bit_matrix.hpp"
#include "../counted.hpp"
#include "../../result_point.hpp"
#include "../../error_handler.hpp"

namespace zxing {

class WhiteRectangleDetector : public Counted {
private:
    static int INIT_SIZE;
    static int CORR;
    Ref<BitMatrix> image_;
    int width_;
    int height_;
    int leftInit_;
    int rightInit_;
    int downInit_;
    int upInit_;
    
public:
    WhiteRectangleDetector(Ref<BitMatrix> image, ErrorHandler & err_handler);
    WhiteRectangleDetector(Ref<BitMatrix> image, int initSize, int x, int y, ErrorHandler & err_handler);
    std::vector<Ref<ResultPoint> > detect(ErrorHandler & err_handler);
    std::vector<Ref<ResultPoint> > detectNew(ErrorHandler & err_handler);
    
private: 
    Ref<ResultPoint> getBlackPointOnSegment(int aX, int aY, int bX, int bY);
    std::vector<Ref<ResultPoint> > centerEdges(Ref<ResultPoint> y, Ref<ResultPoint> z,
                                               Ref<ResultPoint> x, Ref<ResultPoint> t);
    bool containsBlackPoint(int a, int b, int fixed, bool horizontal);
};
}  // namespace zxing

#endif // __ZXING_COMMON_DETECTOR_WHITER_ECTANGLE_DETECTOR_HPP__
