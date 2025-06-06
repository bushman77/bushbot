// C++/WinRT v2.0.220110.5

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once
#ifndef WINRT_Windows_Media_MediaProperties_1_H
#define WINRT_Windows_Media_MediaProperties_1_H
#include "winrt/impl/Windows.Media.MediaProperties.0.h"
WINRT_EXPORT namespace winrt::Windows::Media::MediaProperties
{
    struct __declspec(empty_bases) IAudioEncodingProperties :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IAudioEncodingProperties>,
        impl::require<winrt::Windows::Media::MediaProperties::IAudioEncodingProperties, winrt::Windows::Media::MediaProperties::IMediaEncodingProperties>
    {
        IAudioEncodingProperties(std::nullptr_t = nullptr) noexcept {}
        IAudioEncodingProperties(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IAudioEncodingProperties2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IAudioEncodingProperties2>
    {
        IAudioEncodingProperties2(std::nullptr_t = nullptr) noexcept {}
        IAudioEncodingProperties2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IAudioEncodingProperties3 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IAudioEncodingProperties3>
    {
        IAudioEncodingProperties3(std::nullptr_t = nullptr) noexcept {}
        IAudioEncodingProperties3(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IAudioEncodingPropertiesStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IAudioEncodingPropertiesStatics>
    {
        IAudioEncodingPropertiesStatics(std::nullptr_t = nullptr) noexcept {}
        IAudioEncodingPropertiesStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IAudioEncodingPropertiesStatics2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IAudioEncodingPropertiesStatics2>
    {
        IAudioEncodingPropertiesStatics2(std::nullptr_t = nullptr) noexcept {}
        IAudioEncodingPropertiesStatics2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IAudioEncodingPropertiesWithFormatUserData :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IAudioEncodingPropertiesWithFormatUserData>
    {
        IAudioEncodingPropertiesWithFormatUserData(std::nullptr_t = nullptr) noexcept {}
        IAudioEncodingPropertiesWithFormatUserData(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IAv1ProfileIdsStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IAv1ProfileIdsStatics>
    {
        IAv1ProfileIdsStatics(std::nullptr_t = nullptr) noexcept {}
        IAv1ProfileIdsStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IContainerEncodingProperties :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IContainerEncodingProperties>,
        impl::require<winrt::Windows::Media::MediaProperties::IContainerEncodingProperties, winrt::Windows::Media::MediaProperties::IMediaEncodingProperties>
    {
        IContainerEncodingProperties(std::nullptr_t = nullptr) noexcept {}
        IContainerEncodingProperties(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IContainerEncodingProperties2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IContainerEncodingProperties2>
    {
        IContainerEncodingProperties2(std::nullptr_t = nullptr) noexcept {}
        IContainerEncodingProperties2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IH264ProfileIdsStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IH264ProfileIdsStatics>
    {
        IH264ProfileIdsStatics(std::nullptr_t = nullptr) noexcept {}
        IH264ProfileIdsStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IHevcProfileIdsStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IHevcProfileIdsStatics>
    {
        IHevcProfileIdsStatics(std::nullptr_t = nullptr) noexcept {}
        IHevcProfileIdsStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IImageEncodingProperties :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IImageEncodingProperties>,
        impl::require<winrt::Windows::Media::MediaProperties::IImageEncodingProperties, winrt::Windows::Media::MediaProperties::IMediaEncodingProperties>
    {
        IImageEncodingProperties(std::nullptr_t = nullptr) noexcept {}
        IImageEncodingProperties(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IImageEncodingProperties2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IImageEncodingProperties2>
    {
        IImageEncodingProperties2(std::nullptr_t = nullptr) noexcept {}
        IImageEncodingProperties2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IImageEncodingPropertiesStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IImageEncodingPropertiesStatics>
    {
        IImageEncodingPropertiesStatics(std::nullptr_t = nullptr) noexcept {}
        IImageEncodingPropertiesStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IImageEncodingPropertiesStatics2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IImageEncodingPropertiesStatics2>
    {
        IImageEncodingPropertiesStatics2(std::nullptr_t = nullptr) noexcept {}
        IImageEncodingPropertiesStatics2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IImageEncodingPropertiesStatics3 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IImageEncodingPropertiesStatics3>
    {
        IImageEncodingPropertiesStatics3(std::nullptr_t = nullptr) noexcept {}
        IImageEncodingPropertiesStatics3(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingProfile :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingProfile>
    {
        IMediaEncodingProfile(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingProfile(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingProfile2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingProfile2>
    {
        IMediaEncodingProfile2(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingProfile2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingProfile3 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingProfile3>
    {
        IMediaEncodingProfile3(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingProfile3(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingProfileStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingProfileStatics>
    {
        IMediaEncodingProfileStatics(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingProfileStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingProfileStatics2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingProfileStatics2>
    {
        IMediaEncodingProfileStatics2(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingProfileStatics2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingProfileStatics3 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingProfileStatics3>
    {
        IMediaEncodingProfileStatics3(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingProfileStatics3(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingProfileStatics4 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingProfileStatics4>
    {
        IMediaEncodingProfileStatics4(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingProfileStatics4(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingProperties :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingProperties>
    {
        IMediaEncodingProperties(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingProperties(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingSubtypesStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingSubtypesStatics>
    {
        IMediaEncodingSubtypesStatics(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingSubtypesStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingSubtypesStatics2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingSubtypesStatics2>
    {
        IMediaEncodingSubtypesStatics2(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingSubtypesStatics2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingSubtypesStatics3 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingSubtypesStatics3>
    {
        IMediaEncodingSubtypesStatics3(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingSubtypesStatics3(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingSubtypesStatics4 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingSubtypesStatics4>
    {
        IMediaEncodingSubtypesStatics4(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingSubtypesStatics4(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingSubtypesStatics5 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingSubtypesStatics5>
    {
        IMediaEncodingSubtypesStatics5(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingSubtypesStatics5(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingSubtypesStatics6 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingSubtypesStatics6>
    {
        IMediaEncodingSubtypesStatics6(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingSubtypesStatics6(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaEncodingSubtypesStatics7 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaEncodingSubtypesStatics7>
    {
        IMediaEncodingSubtypesStatics7(std::nullptr_t = nullptr) noexcept {}
        IMediaEncodingSubtypesStatics7(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMediaRatio :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMediaRatio>
    {
        IMediaRatio(std::nullptr_t = nullptr) noexcept {}
        IMediaRatio(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IMpeg2ProfileIdsStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMpeg2ProfileIdsStatics>
    {
        IMpeg2ProfileIdsStatics(std::nullptr_t = nullptr) noexcept {}
        IMpeg2ProfileIdsStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) ITimedMetadataEncodingProperties :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<ITimedMetadataEncodingProperties>
    {
        ITimedMetadataEncodingProperties(std::nullptr_t = nullptr) noexcept {}
        ITimedMetadataEncodingProperties(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) ITimedMetadataEncodingPropertiesStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<ITimedMetadataEncodingPropertiesStatics>
    {
        ITimedMetadataEncodingPropertiesStatics(std::nullptr_t = nullptr) noexcept {}
        ITimedMetadataEncodingPropertiesStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVideoEncodingProperties :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVideoEncodingProperties>,
        impl::require<winrt::Windows::Media::MediaProperties::IVideoEncodingProperties, winrt::Windows::Media::MediaProperties::IMediaEncodingProperties>
    {
        IVideoEncodingProperties(std::nullptr_t = nullptr) noexcept {}
        IVideoEncodingProperties(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVideoEncodingProperties2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVideoEncodingProperties2>
    {
        IVideoEncodingProperties2(std::nullptr_t = nullptr) noexcept {}
        IVideoEncodingProperties2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVideoEncodingProperties3 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVideoEncodingProperties3>
    {
        IVideoEncodingProperties3(std::nullptr_t = nullptr) noexcept {}
        IVideoEncodingProperties3(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVideoEncodingProperties4 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVideoEncodingProperties4>
    {
        IVideoEncodingProperties4(std::nullptr_t = nullptr) noexcept {}
        IVideoEncodingProperties4(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVideoEncodingProperties5 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVideoEncodingProperties5>
    {
        IVideoEncodingProperties5(std::nullptr_t = nullptr) noexcept {}
        IVideoEncodingProperties5(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVideoEncodingPropertiesStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVideoEncodingPropertiesStatics>
    {
        IVideoEncodingPropertiesStatics(std::nullptr_t = nullptr) noexcept {}
        IVideoEncodingPropertiesStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVideoEncodingPropertiesStatics2 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVideoEncodingPropertiesStatics2>
    {
        IVideoEncodingPropertiesStatics2(std::nullptr_t = nullptr) noexcept {}
        IVideoEncodingPropertiesStatics2(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVideoEncodingPropertiesStatics3 :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVideoEncodingPropertiesStatics3>
    {
        IVideoEncodingPropertiesStatics3(std::nullptr_t = nullptr) noexcept {}
        IVideoEncodingPropertiesStatics3(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IVp9ProfileIdsStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IVp9ProfileIdsStatics>
    {
        IVp9ProfileIdsStatics(std::nullptr_t = nullptr) noexcept {}
        IVp9ProfileIdsStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
}
#endif
