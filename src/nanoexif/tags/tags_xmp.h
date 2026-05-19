// SPDX-License-Identifier: MIT

#pragma once

#include <string_view>

namespace nne {

namespace XmpNs {

// DC (Dublin Core)
inline constexpr std::string_view dc = "dc";
inline constexpr std::string_view dc_creator = "dc:creator";
inline constexpr std::string_view dc_title = "dc:title";
inline constexpr std::string_view dc_description = "dc:description";
inline constexpr std::string_view dc_subject = "dc:subject";
inline constexpr std::string_view dc_date = "dc:date";
inline constexpr std::string_view dc_format = "dc:format";
inline constexpr std::string_view dc_publisher = "dc:publisher";
inline constexpr std::string_view dc_rights = "dc:rights";
inline constexpr std::string_view dc_language = "dc:language";
inline constexpr std::string_view dc_type = "dc:type";
inline constexpr std::string_view dc_identifier = "dc:identifier";
inline constexpr std::string_view dc_contributor = "dc:contributor";
inline constexpr std::string_view dc_coverage = "dc:coverage";
inline constexpr std::string_view dc_relation = "dc:relation";
inline constexpr std::string_view dc_source = "dc:source";

// XMP basic
inline constexpr std::string_view xmp = "xmp";
inline constexpr std::string_view xmp_CreateDate = "xmp:CreateDate";
inline constexpr std::string_view xmp_ModifyDate = "xmp:ModifyDate";
inline constexpr std::string_view xmp_MetadataDate = "xmp:MetadataDate";
inline constexpr std::string_view xmp_Rating = "xmp:Rating";
inline constexpr std::string_view xmp_Label = "xmp:Label";
inline constexpr std::string_view xmp_CreatorTool = "xmp:CreatorTool";

// EXIF in XMP
inline constexpr std::string_view exif = "exif";
inline constexpr std::string_view exif_FNumber = "exif:FNumber";
inline constexpr std::string_view exif_ExposureTime = "exif:ExposureTime";
inline constexpr std::string_view exif_ISOSpeedRatings =
    "exif:ISOSpeedRatings";
inline constexpr std::string_view exif_FocalLength = "exif:FocalLength";
inline constexpr std::string_view exif_ExposureProgram =
    "exif:ExposureProgram";
inline constexpr std::string_view exif_DateTimeOriginal =
    "exif:DateTimeOriginal";

// TIFF in XMP
inline constexpr std::string_view tiff = "tiff";
inline constexpr std::string_view tiff_Make = "tiff:Make";
inline constexpr std::string_view tiff_Model = "tiff:Model";
inline constexpr std::string_view tiff_Orientation = "tiff:Orientation";
inline constexpr std::string_view tiff_XResolution = "tiff:XResolution";
inline constexpr std::string_view tiff_YResolution = "tiff:YResolution";

// IPTC
inline constexpr std::string_view iptc = "Iptc";
inline constexpr std::string_view photoshop = "photoshop";
inline constexpr std::string_view photoshop_Credit = "photoshop:Credit";
inline constexpr std::string_view photoshop_DateCreated =
    "photoshop:DateCreated";

// Camera Raw
inline constexpr std::string_view crs = "crs";

// RDF
inline constexpr std::string_view rdf = "rdf";

// XMP Media Management
inline constexpr std::string_view xmpMM = "xmpMM";
inline constexpr std::string_view xmpMM_DocumentID = "xmpMM:DocumentID";
inline constexpr std::string_view xmpMM_InstanceID = "xmpMM:InstanceID";
inline constexpr std::string_view xmpMM_OriginalDocumentID =
    "xmpMM:OriginalDocumentID";

}  // namespace XmpNs
}  // namespace nne
