// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "slice.h"

namespace ncnn {

DEFINE_LAYER_CREATOR(Slice)

Slice::Slice()
{
}

int Slice::load_param(const ParamDict& pd)
{
    slices = pd.get(0, Mat());
    axis = pd.get(1, 0);

    return 0;
}

int Slice::forward(const std::vector<Mat>& bottom_blobs, std::vector<Mat>& top_blobs) const
{
    const Mat& bottom_blob = bottom_blobs[0];
    int dims = bottom_blob.dims;
    const int* slices_ptr = slices;

    if (dims == 1) // axis == 0
    {
        int w = bottom_blob.w;

        int q = 0;
        for (size_t i=0; i<top_blobs.size(); i++)
        {
            int slice = slices_ptr[i];
            if (slice == -233)
            {
                slice = (w - q) / (top_blobs.size() - i);
            }

            Mat& top_blob = top_blobs[i];
            top_blob.create(slice);
            if (top_blob.empty())
                return -100;

            const float* ptr = (const float*)bottom_blob + q;
            float* outptr = top_blob;
            memcpy(outptr, ptr, slice * sizeof(float));

            q += slice;
        }

        return 0;
    }

    if (dims == 2 && axis == 0)
    {
        int w = bottom_blob.w;
        int h = bottom_blob.h;

        int q = 0;
        for (size_t i=0; i<top_blobs.size(); i++)
        {
            int slice = slices_ptr[i];
            if (slice == -233)
            {
                slice = (h - q) / (top_blobs.size() - i);
            }

            Mat& top_blob = top_blobs[i];
            top_blob.create(w, slice);
            if (top_blob.empty())
                return -100;

            int size = w * slice;

            const float* ptr = bottom_blob.row(q);
            float* outptr = top_blob;
            memcpy(outptr, ptr, size * sizeof(float));

            q += slice;
        }

        return 0;
    }

    if (dims == 2 && axis == 1)
    {
        int w = bottom_blob.w;
        int h = bottom_blob.h;

        int q = 0;
        for (size_t i=0; i<top_blobs.size(); i++)
        {
            int slice = slices_ptr[i];
            if (slice == -233)
            {
                slice = (w - q) / (top_blobs.size() - i);
            }

            Mat& top_blob = top_blobs[i];
            top_blob.create(slice, h);
            if (top_blob.empty())
                return -100;

            #pragma omp parallel for
            for (int j=0; j<h; j++)
            {
                float* outptr = top_blob.row(j);
                const float* ptr = bottom_blob.row(j) + q;
                memcpy(outptr, ptr, slice * sizeof(float));
            }

            q += slice;
        }

        return 0;
    }

    if (dims == 3 && axis == 0)
    {
        int w = bottom_blob.w;
        int h = bottom_blob.h;
        int channels = bottom_blob.c;

        int q = 0;
        for (size_t i=0; i<top_blobs.size(); i++)
        {
            int slice = slices_ptr[i];
            if (slice == -233)
            {
                slice = (channels - q) / (top_blobs.size() - i);
            }

            Mat& top_blob = top_blobs[i];
            top_blob.create(w, h, slice);
            if (top_blob.empty())
                return -100;

            int size = w * h * slice;

            const float* ptr = bottom_blob.channel(q);
            float* outptr = top_blob;
            memcpy(outptr, ptr, size * sizeof(float));

            q += slice;
        }

        return 0;
    }

    if (dims == 3 && axis == 1)
    {
        int w = bottom_blob.w;
        int h = bottom_blob.h;
        int channels = bottom_blob.c;

        int q = 0;
        for (size_t i=0; i<top_blobs.size(); i++)
        {
            int slice = slices_ptr[i];
            if (slice == -233)
            {
                slice = (h - q) / (top_blobs.size() - i);
            }

            Mat& top_blob = top_blobs[i];
            top_blob.create(w, slice, channels);
            if (top_blob.empty())
                return -100;

            #pragma omp parallel for
            for (int p=0; p<channels; p++)
            {
                int size = w * slice;

                float* outptr = top_blob.channel(p);
                const float* ptr = bottom_blob.channel(p).row(q);
                memcpy(outptr, ptr, size * sizeof(float));
            }

            q += slice;
        }

        return 0;
    }

    if (dims == 3 && axis == 2)
    {
        int w = bottom_blob.w;
        int h = bottom_blob.h;
        int channels = bottom_blob.c;

        int q = 0;
        for (size_t i=0; i<top_blobs.size(); i++)
        {
            int slice = slices_ptr[i];
            if (slice == -233)
            {
                slice = (w - q) / (top_blobs.size() - i);
            }

            Mat& top_blob = top_blobs[i];
            top_blob.create(slice, h, channels);
            if (top_blob.empty())
                return -100;

            #pragma omp parallel for
            for (int p=0; p<channels; p++)
            {
                float* outptr = top_blob.channel(p);
                const Mat m = bottom_blob.channel(p);

                for (int j=0; j<h; j++)
                {
                    const float* ptr = m.row(j) + q;
                    memcpy(outptr, ptr, slice * sizeof(float));

                    outptr += slice;
                }
            }

            q += slice;
        }

        return 0;
    }

    return 0;
}

} // namespace ncnn
