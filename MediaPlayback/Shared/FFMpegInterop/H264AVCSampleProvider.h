//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once
#include "MediaSampleProvider.h"

namespace FFmpegInterop
{
    class H264AVCSampleProvider :
        public MediaSampleProvider
    {
    public:
        H264AVCSampleProvider(
            _In_ std::weak_ptr<FFmpegReader> reader,
            _In_ AVFormatContext* avFormatCtx,
            _In_ AVCodecContext* avCodecCtx);
        virtual ~H264AVCSampleProvider();

    private:
        HRESULT WriteNALPacket(
            _In_ ABI::Windows::Storage::Streams::IDataWriter* dataWriter, 
            _In_ AVPacket* avPacket);
        HRESULT GetSPSAndPPSBuffer(
            _In_ ABI::Windows::Storage::Streams::IDataWriter* dataWriter);

        virtual HRESULT WriteAVPacketToStream(
            _In_ ABI::Windows::Storage::Streams::IDataWriter* writer, 
            _In_ AVPacket* avPacket) override;
    };
}
