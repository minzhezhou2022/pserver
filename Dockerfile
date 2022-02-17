FROM nvidia/cuda:11.2.1-cudnn8-devel-ubuntu18.04

RUN mkdir -p /homura
RUN mkdir -p /usr/local/third_party
ENV THIRD_PARTY_DIR="/usr/local/third_party"

ARG BUILD_JOBS=32
ENV BUILD_JOBS ${BUILD_JOBS}

ENV DEBIAN_FRONTEND noninteractive
RUN ln -fs /usr/share/zoneinfo/America/Pacific /etc/localtime

ENV NCCL_VERSION=2.8.4-1+cuda11.2

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        bison \
        build-essential \
        curl \
        dh-autoreconf \
        g++-8 \
        gcc-8 \
        gfortran \
        git \
        google-perftools \
        graphviz \
        libatlas-base-dev \
        libcairomm-1.0-dev \
        libexiv2-dev \
        libffi-dev \
        libgflags-dev \
        libgmp3-dev libcgal-dev \
        libgoogle-glog-dev \
        libgtest-dev \
        libhdf5-serial-dev \
        libiomp-dev \
        libjpeg8-dev \
        libkrb5-dev \
        libleveldb-dev \
        liblmdb-dev \
        liblz4-dev \
        libmagickwand-dev \
        libmysqlclient-dev \
        libnccl-dev=${NCCL_VERSION} \
        libnccl2=${NCCL_VERSION} \
        libopenblas-dev \
        libopencv-dev \
        libopenmpi-dev \
        libreadline-dev \
        libsnappy-dev \
        libsparsehash-dev \
        libsqlite3-dev \
        libssl-dev \
        libwebp-dev \
        libxml2-dev \
        libxslt1-dev \
        numactl \
        openjdk-8-jdk \
        openmpi-bin \
        openmpi-doc \
        python-dev \
        python-lxml \
        python-tk \
        python3-setuptools \
        tk-dev \
        tzdata \
        unzip \
        vim \
        wget \
        zip \
        zlib1g-dev \
        ssh \
 && dpkg-reconfigure --frontend noninteractive tzdata

RUN curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add -

# Install cmake3.21
# renovate: datasource=github-releases depName=Kitware/CMake
ARG CMAKE_VERSION=3.21.0
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && mkdir /usr/bin/cmake \
      && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
      && rm /tmp/cmake-install.sh
ENV PATH="/usr/bin/cmake/bin:${PATH}"


# Install python 3.7 (default is 3.5)
RUN cd /opt && \
    wget https://www.python.org/ftp/python/3.7.5/Python-3.7.5.tgz && \
    tar -xvf Python-3.7.5.tgz && \
    (cd Python-3.7.5 && \
      ./configure --enable-optimizations --enable-shared --enable-loadable-sqlite-extensions && \
      make altinstall && \
      ln -s /usr/local/include/python3.7m /usr/local/include/python3.7 && \
      ldconfig) && \
    rm -rf Python-3.7.5.tgz Python-3.7.5

RUN alias pip3.7="python3.7 -m pip"

# Upgrade pip and setuptools
RUN pip3.7 install --upgrade pip==20.1.1 setuptools==45.3.0

# Need a newer version of swig for faiss
RUN cd /opt && \
    wget https://github.com/swig/swig/archive/rel-4.0.1.zip && \
    unzip rel-4.0.1.zip && \
    (cd swig-rel-4.0.1/ && \
      ./autogen.sh && \
      ./configure --with-python3=python3.7 && \
      make -j ${BUILD_JOBS} && \
      make -j ${BUILD_JOBS} install && \
      ldconfig) && \
    rm -rf rel-4.0.1.zip swig-rel-4.0.1/

# Build boost (need to manually do for python3.7 on ubuntu18.04)
RUN cd /opt && \
    wget -O boost_1_67_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.67.0/boost_1_67_0.tar.gz/download && \
    tar -xvf boost_1_67_0.tar.gz && \
    (cd boost_1_67_0/ && \
      ./bootstrap.sh --with-python-version=3.7 --with-python-root=/usr/local/ && \
      ./b2 install threading=multi link=shared -j ${BUILD_JOBS} && \
      ldconfig) && \
    rm -rf boost*

RUN pip3.7 install \
    h5py==2.8.0 \
    cython==0.29.21 \
    numpy==1.19.4

# Build lib_detection modules, eg Cython modules (nms, bbox_overlaps)
RUN pip3.7 install \
    cython==0.29.21 \
    numpy==1.19.4

ENV DETECTION_LIBRARY_ROOT=/opt/detection_library
COPY detection_library ${DETECTION_LIBRARY_ROOT}
RUN cd ${DETECTION_LIBRARY_ROOT} \
    && cd lib_detection \
    && make -j ${BUILD_JOBS}
ENV PYTHONPATH=$PYTHONPATH:${DETECTION_LIBRARY_ROOT}/

# mkl
RUN pip3.7 install mkl-devel==2021.1.1

# Caffe2 + Pytorch
ENV PYTORCH_ROOT=/opt/pytorch

# Check out v1.9.0 release
RUN cd /opt && \
    git clone https://github.com/pytorch/pytorch.git && \
    cd pytorch && \
    git checkout v1.9.0 && \
    git submodule update --init --recursive


RUN cd $PYTORCH_ROOT \
    && sed -i 's$option(USE_NATIVE_ARCH "Use -march=native" OFF)$option(USE_NATIVE_ARCH "Use -march=native" ON)$g' CMakeLists.txt \
    && sed -i 's$caffe2_binary_target("convert_caffe_image_db.cc")$#caffe2_binary_target("convert_caffe_image_db.cc")$g' binaries/CMakeLists.txt \
    && pip3.7 install -r requirements.txt \
    && TORCH_CUDA_ARCH_LIST="5.2 6.0 6.1 7.0+PTX 7.5 8.0" TORCH_NVCC_FLAGS="-Xfatbin -compress-all" \
    CMAKE_INCLUDE_PATH=${CAFFE_ROOT}/build/src CMAKE_LIBRARY_PATH=$LD_LIBRARY_PATH \
    USE_OPENCV=1 BUILD_BINARY=1 BUILD_TEST=0 USE_MKL=1 USE_MKLDNN_CBLAS=1 USE_MKLDNN=1 GLIBCXX_USE_CXX11_ABI=1 \
    NCCL_INCLUDE_DIR="/usr/include/" NCCL_LIB_DIR="/usr/lib/" USE_SYSTEM_NCCL=1 python3.7 setup.py install \
    && cd / && rm -rf ${PYTORCH_ROOT}

# Change the gcc/g++ aliases to point to 8; required by pip install torchscript
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8


# Install zeromq
RUN cd /opt && \
    wget https://archive.org/download/zeromq_3.2.5/zeromq-3.2.5.zip && \
    unzip zeromq-3.2.5.zip && \
    cd zeromq-3.2.5 && \
        mkdir -p ${THIRD_PARTY_DIR}/zeromq && \
        ./configure --prefix=${THIRD_PARTY_DIR}/zeromq && \
        make -j 4 && make install && \
    cd - && rm -rf zeromq-3.2.5    
        
    
# Install yaml
RUN cd /opt && \
    wget http://sources.buildroot.net/yaml-cpp/yaml-cpp-0.6.3.tar.gz && \
    tar -zxf yaml-cpp-0.6.3.tar.gz && \
    cd yaml-cpp-yaml-cpp-0.6.3 && \
        mkdir -p ${THIRD_PARTY_DIR}/yaml && \
        mkdir build && cd build && cmake -DCMAKE_INSTALL_PREFIX:PATH=${THIRD_PARTY_DIR}/yaml .. && \
        make -j4 && make install && \
    cd - && rm -rf yaml-cpp-0.6.3.tar.gz yaml-cpp-yaml-cpp-0.6.3

# Install gtest
RUN cd /opt && \
    wget https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz && \
    tar -zxf release-1.11.0.tar.gz && \
    mkdir -p googletest-release-1.11.0/build && \
    cd googletest-release-1.11.0/build && \
        mkdir -p ${THIRD_PARTY_DIR}/gtest && \
        cmake -DCMAKE_INSTALL_PREFIX:PATH=${THIRD_PARTY_DIR}/gtest .. && \
        make -j4 && make install && \
    cd - && rm -rf googletest-release-1.11.0

# Install aws
RUN cd /opt && \
    curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip" && \
    unzip awscliv2.zip && \
    ./aws/install && \
    rm -rf awscliv2.zip aws

WORKDIR /homura
