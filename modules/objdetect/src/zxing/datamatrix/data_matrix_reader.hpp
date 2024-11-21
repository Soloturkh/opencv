// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Tencent is pleased to support the open source community by making WeChat QRCode available.
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Modified from ZXing. Copyright ZXing authors.
// Licensed under the Apache License, Version 2.0 (the "License").

#ifndef __ZXING_DATAMATRIX_DATA_MATRIX_READER_HPP__
#define __ZXING_DATAMATRIX_DATA_MATRIX_READER_HPP__

/*
 *  DataMatrixReader.hpp
 *  zxing
 *
 *  Created by Luiz Silva on 09/02/2010.
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

#include "../reader.hpp"
#include "../decode_hints.hpp"
#include "decoder/decoder.hpp"
#include "../error_handler.hpp"

namespace zxing {
namespace datamatrix {

class DataMatrixReader : public Reader {
private:
    Decoder decoder_;
    
public:
    DataMatrixReader();
    std::string name() { return "datamatrix"; }
    virtual Ref<Result> decode(Ref<BinaryBitmap> image, DecodeHints hints);
    Ref<Result> decodeMore(Ref<BinaryBitmap> image, Ref<BitMatrix> imageBitMatrix, DecodeHints hints, ErrorHandler & err_handler);
    virtual ~DataMatrixReader();
};

}  // namespace datamatrix
}  // namespace zxing

#endif  // __ZXING_DATAMATRIX_DATA_MATRIX_READER_HPP__
