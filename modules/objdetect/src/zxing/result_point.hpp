// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Tencent is pleased to support the open source community by making WeChat QRCode available.
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Modified from ZXing. Copyright ZXing authors.
// Licensed under the Apache License, Version 2.0 (the "License").

// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef __ZXING_RESULT_POINT_HPP__
#define __ZXING_RESULT_POINT_HPP__

/*
 *  ResultPoint.hpp
 *  zxing
 *
 *  Copyright 2010 ZXing authors All rights reserved.
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

#include "common/counted.hpp"
#include <vector>

namespace zxing {

class ResultPoint : public Counted {
protected:
    float posX_;
    float posY_;
    
public:
    ResultPoint();
    ResultPoint(float x, float y);
    ResultPoint(int x, int y);
    virtual ~ResultPoint();
    
    virtual float getX() const;
    virtual float getY() const;
    virtual void setX(float fX);
    virtual void setY(float fY);
    
    bool equals(Ref<ResultPoint> other);
    
    static void orderBestPatterns(std::vector<Ref<ResultPoint> > &patterns);
    static float distance(Ref<ResultPoint> point1, Ref<ResultPoint> point2);
    static float distance(float x1, float x2, float y1, float y2);
    
private:
    static float crossProductZ(Ref<ResultPoint> pointA, Ref<ResultPoint> pointB, Ref<ResultPoint> pointC);
};

}  // namespace zxing

#endif  // __ZXING_RESULT_POINT_HPP__
