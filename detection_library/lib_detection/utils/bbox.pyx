# --------------------------------------------------------
# Fast R-CNN
# Copyright (c) 2015 Microsoft
# Licensed under The MIT License [see LICENSE for details]
# Written by Sergey Karayev
# --------------------------------------------------------

cimport cython
import numpy as np
cimport numpy as np

DTYPE = np.float
ctypedef np.float_t DTYPE_t

def bbox_overlaps(
        np.ndarray[DTYPE_t, ndim=2] boxes,
        np.ndarray[DTYPE_t, ndim=2] query_boxes):
    """
    Parameters
    ----------
    boxes: (N, 4) ndarray of float
        [x1, y1, x2, y2], in pixel or normalized coordinates
    query_boxes: (K, 4) ndarray of float
        [x1, y1, x2, y2], in pixel or normalized coordinates
    Returns
    -------
    overlaps: (N, K) ndarray of overlap between boxes and query_boxes
        Example: overlaps[box_i, query_i] returns the iou overlap between
            boxes[box_i, :] and query_boxes[query_i, :]
    """
    # Note(erickim)[July 16 2018] This originally operated strictly on pixel
    # coords, ie `w = x2 - x1 + 1`. I removed the +-1 offsets for two reasons:
    # (1) MS COCO computes IOU without the +-1 offsets:
    #     https://github.com/cocodataset/cocoapi/blob/ff4a47150bf666762fa3454335453520e7a041a8/common/maskApi.c
    # (2) Have a single bbox_overlaps() function that works on both pixel coords
    #     and normalized coords.
    # It's worth noting that, for images larger than [100 x 100] dimensions, the
    # computed IOU difference between using +-1 offset or without is negligible.
    cdef unsigned int N = boxes.shape[0]
    cdef unsigned int K = query_boxes.shape[0]
    cdef np.ndarray[DTYPE_t, ndim=2] overlaps = np.zeros((N, K), dtype=DTYPE)
    cdef DTYPE_t iw, ih, box_area
    cdef DTYPE_t ua
    cdef unsigned int k, n
    for k in range(K):
        box_area = (
            (query_boxes[k, 2] - query_boxes[k, 0]) *
            (query_boxes[k, 3] - query_boxes[k, 1])
        )
        for n in range(N):
            iw = (
                min(boxes[n, 2], query_boxes[k, 2]) -
                max(boxes[n, 0], query_boxes[k, 0])
            )
            if iw > 0:
                ih = (
                    min(boxes[n, 3], query_boxes[k, 3]) -
                    max(boxes[n, 1], query_boxes[k, 1])
                )
                if ih > 0:
                    ua = float(
                        (boxes[n, 2] - boxes[n, 0]) *
                        (boxes[n, 3] - boxes[n, 1]) +
                        box_area - iw * ih
                    )
                    overlaps[n, k] = iw * ih / ua
    return overlaps
