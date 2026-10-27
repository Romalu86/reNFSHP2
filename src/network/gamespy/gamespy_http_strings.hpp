#pragma once
namespace nfs::network::session::pending::http {

inline constexpr char multipart_content_type[] =
    "multipart/form-data; boundary=Qr4G823s23d---<<><><<<>--7d118e0536";

inline constexpr char urlencoded_content_type[] = "application/x-www-form-urlencoded";

inline constexpr char request_version[] = " HTTP/1.1\r\n";

inline constexpr char status_format[] = "HTTP/%d.%d %d%n";

inline constexpr char content_length_header[] = "Content-Length:";

inline constexpr char chunked_header[] = "Transfer-Encoding: chunked";

inline constexpr char location_header[] = "Location:";

inline constexpr char boundary[] = "--Qr4G823s23d---<<><><<<>--7d118e0536";

inline constexpr char closing_boundary[] = "\r\n--Qr4G823s23d---<<><><<<>--7d118e0536--\r\n";

inline constexpr char next_boundary[] = "\r\n--Qr4G823s23d---<<><><<<>--7d118e0536\r\n";

inline constexpr char first_boundary[] = "--Qr4G823s23d---<<><><<<>--7d118e0536\r\n";

inline constexpr char form_field_format[] = "%sContent-Disposition: form-data; name=\"%s\"\r\n\r\n";

inline constexpr char form_file_format[] =
    "%sContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: %s\r\n\r\n";

inline constexpr char first_url_field_format[] = "%s=";

inline constexpr char next_url_field_format[] = "&%s=";

inline constexpr char url_safe_characters[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_@-.*";
} // namespace nfs::network::session::pending::http
