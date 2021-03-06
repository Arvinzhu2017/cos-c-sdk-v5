#include "cos_log.h"
#include "cos_sys_util.h"
#include "cos_string.h"
#include "cos_status.h"
#include "cos_auth.h"
#include "cos_utility.h"
#include "cos_xml.h"
#include "cos_api.h"

cos_status_t *cos_put_object_from_buffer(const cos_request_options_t *options,
                                         const cos_string_t *bucket, 
                                         const cos_string_t *object, 
                                         cos_list_t *buffer,
                                         cos_table_t *headers, 
                                         cos_table_t **resp_headers)
{
    return cos_do_put_object_from_buffer(options, bucket, object, buffer, 
                                         headers, NULL, NULL, resp_headers, NULL);
}

cos_status_t *cos_do_put_object_from_buffer(const cos_request_options_t *options,
                                            const cos_string_t *bucket, 
                                            const cos_string_t *object, 
                                            cos_list_t *buffer,
                                            cos_table_t *headers, 
                                            cos_table_t *params,
                                            cos_progress_callback progress_callback,
                                            cos_table_t **resp_headers,
                                            cos_list_t *resp_body)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;

    headers = cos_table_create_if_null(options, headers, 2);
    set_content_type(NULL, object->data, headers);
    apr_table_add(headers, COS_EXPECT, "");

    query_params = cos_table_create_if_null(options, params, 0);

    cos_init_object_request(options, bucket, object, HTTP_PUT, 
                            &req, query_params, headers, progress_callback, 0, &resp);
    cos_write_request_body_from_buffer(buffer, req);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_body(resp, resp_body);
    cos_fill_read_response_header(resp, resp_headers);

    if (is_enable_crc(options) && has_crc_in_response(resp)) {
        cos_check_crc_consistent(req->crc64, resp->headers, s);
    }

    return s;
}

cos_status_t *cos_put_object_from_file(const cos_request_options_t *options,
                                       const cos_string_t *bucket, 
                                       const cos_string_t *object, 
                                       const cos_string_t *filename,
                                       cos_table_t *headers, 
                                       cos_table_t **resp_headers)
{
    return cos_do_put_object_from_file(options, bucket, object, filename, 
                                       headers, NULL, NULL, resp_headers, NULL);
}

cos_status_t *cos_do_put_object_from_file(const cos_request_options_t *options,
                                          const cos_string_t *bucket, 
                                          const cos_string_t *object, 
                                          const cos_string_t *filename,
                                          cos_table_t *headers, 
                                          cos_table_t *params,
                                          cos_progress_callback progress_callback,
                                          cos_table_t **resp_headers,
                                          cos_list_t *resp_body)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    int res = COSE_OK;

    s = cos_status_create(options->pool);

    headers = cos_table_create_if_null(options, headers, 2);
    set_content_type(filename->data, object->data, headers);
    apr_table_add(headers, COS_EXPECT, "");

    query_params = cos_table_create_if_null(options, params, 0);

    cos_init_object_request(options, bucket, object, HTTP_PUT, &req, 
                            query_params, headers, progress_callback, 0, &resp);

    res = cos_write_request_body_from_file(options->pool, filename, req);
    if (res != COSE_OK) {
        cos_file_error_status_set(s, res);
        return s;
    }

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_body(resp, resp_body);
    cos_fill_read_response_header(resp, resp_headers);

    if (is_enable_crc(options) && has_crc_in_response(resp)) {
        cos_check_crc_consistent(req->crc64, resp->headers, s);
    }

    return s;
}

cos_status_t *cos_get_object_to_buffer(const cos_request_options_t *options, 
                                       const cos_string_t *bucket, 
                                       const cos_string_t *object,
                                       cos_table_t *headers, 
                                       cos_table_t *params,
                                       cos_list_t *buffer, 
                                       cos_table_t **resp_headers)
{
    return cos_do_get_object_to_buffer(options, bucket, object, headers, 
                                       params, buffer, NULL, resp_headers);
}

cos_status_t *cos_do_get_object_to_buffer(const cos_request_options_t *options, 
                                          const cos_string_t *bucket, 
                                          const cos_string_t *object,
                                          cos_table_t *headers, 
                                          cos_table_t *params,
                                          cos_list_t *buffer,
                                          cos_progress_callback progress_callback, 
                                          cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;

    headers = cos_table_create_if_null(options, headers, 0);
    params = cos_table_create_if_null(options, params, 0);

    cos_init_object_request(options, bucket, object, HTTP_GET, 
                            &req, params, headers, progress_callback, 0, &resp);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_body(resp, buffer);
    cos_fill_read_response_header(resp, resp_headers);

    if (is_enable_crc(options) && has_crc_in_response(resp) &&  
        !has_range_or_process_in_request(req)) {
        cos_check_crc_consistent(resp->crc64, resp->headers, s);
    }

    return s;
}

cos_status_t *cos_get_object_to_file(const cos_request_options_t *options,
                                     const cos_string_t *bucket, 
                                     const cos_string_t *object,
                                     cos_table_t *headers, 
                                     cos_table_t *params,
                                     cos_string_t *filename, 
                                     cos_table_t **resp_headers)
{
    return cos_do_get_object_to_file(options, bucket, object, headers, 
                                     params, filename, NULL, resp_headers);
}

cos_status_t *cos_do_get_object_to_file(const cos_request_options_t *options,
                                        const cos_string_t *bucket, 
                                        const cos_string_t *object,
                                        cos_table_t *headers, 
                                        cos_table_t *params,
                                        cos_string_t *filename, 
                                        cos_progress_callback progress_callback,
                                        cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    int res = COSE_OK;
    cos_string_t tmp_filename;

    headers = cos_table_create_if_null(options, headers, 0);
    params = cos_table_create_if_null(options, params, 0);

    cos_get_temporary_file_name(options->pool, filename, &tmp_filename);

    cos_init_object_request(options, bucket, object, HTTP_GET, 
                            &req, params, headers, progress_callback, 0, &resp);

    s = cos_status_create(options->pool);
    res = cos_init_read_response_body_to_file(options->pool, &tmp_filename, resp);
    if (res != COSE_OK) {
        cos_file_error_status_set(s, res);
        return s;
    }

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    if (is_enable_crc(options) && has_crc_in_response(resp) && 
        !has_range_or_process_in_request(req)) {
            cos_check_crc_consistent(resp->crc64, resp->headers, s);
    }

    cos_temp_file_rename(s, tmp_filename.data, filename->data, options->pool);

    return s;
}

cos_status_t *cos_head_object(const cos_request_options_t *options, 
                              const cos_string_t *bucket, 
                              const cos_string_t *object,
                              cos_table_t *headers, 
                              cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;

    headers = cos_table_create_if_null(options, headers, 0);    

    query_params = cos_table_create_if_null(options, query_params, 0);

    cos_init_object_request(options, bucket, object, HTTP_HEAD, 
                            &req, query_params, headers, NULL, 0, &resp);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    return s;
}

cos_status_t *cos_delete_object(const cos_request_options_t *options,
                                const cos_string_t *bucket, 
                                const cos_string_t *object, 
                                cos_table_t **resp_headers)
{
    return cos_do_delete_object(options, bucket, object, NULL, resp_headers);
}

cos_status_t *cos_do_delete_object(const cos_request_options_t *options,
                                const cos_string_t *bucket, 
                                const cos_string_t *object,
                                cos_table_t *headers, 
                                cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *pHeaders = NULL;
    cos_table_t *query_params = NULL;

    pHeaders = cos_table_create_if_null(options, headers, 0);
    query_params = cos_table_create_if_null(options, query_params, 0);

    cos_init_object_request(options, bucket, object, HTTP_DELETE, 
                            &req, query_params, pHeaders, NULL, 0, &resp);
    cos_get_object_uri(options, bucket, object, req);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    return s;
}


cos_status_t *cos_append_object_from_buffer(const cos_request_options_t *options,
                                            const cos_string_t *bucket, 
                                            const cos_string_t *object, 
                                            int64_t position,
                                            cos_list_t *buffer, 
                                            cos_table_t *headers, 
                                            cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    
    /* init query_params */
    query_params = cos_table_create_if_null(options, query_params, 2);
    apr_table_add(query_params, COS_APPEND, "");
    cos_table_add_int64(query_params, COS_POSITION, position);

    /* init headers */
    headers = cos_table_create_if_null(options, headers, 2);
    set_content_type(NULL, object->data, headers);
    apr_table_add(headers, COS_EXPECT, "");

    cos_init_object_request(options, bucket, object, HTTP_POST, 
                            &req, query_params, headers, NULL, 0, &resp);
    cos_write_request_body_from_buffer(buffer, req);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    return s;
}

cos_status_t *cos_do_append_object_from_buffer(const cos_request_options_t *options,
                                               const cos_string_t *bucket, 
                                               const cos_string_t *object, 
                                               int64_t position,
                                               uint64_t init_crc,
                                               cos_list_t *buffer, 
                                               cos_table_t *headers,
                                               cos_table_t *params,
                                               cos_progress_callback progress_callback,
                                               cos_table_t **resp_headers,
                                               cos_list_t *resp_body)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    
    /* init query_params */
    query_params = cos_table_create_if_null(options, params, 2);
    apr_table_add(query_params, COS_APPEND, "");
    cos_table_add_int64(query_params, COS_POSITION, position);

    /* init headers */
    headers = cos_table_create_if_null(options, headers, 2);
    set_content_type(NULL, object->data, headers);
    apr_table_add(headers, COS_EXPECT, "");

    cos_init_object_request(options, bucket, object, HTTP_POST, &req, query_params, 
                            headers, progress_callback, init_crc, &resp);
    cos_write_request_body_from_buffer(buffer, req);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);
    cos_fill_read_response_body(resp, resp_body);

    if (is_enable_crc(options) && has_crc_in_response(resp)) {
        cos_check_crc_consistent(req->crc64, resp->headers, s);
    }

    return s;
}

cos_status_t *cos_append_object_from_file(const cos_request_options_t *options,
                                          const cos_string_t *bucket, 
                                          const cos_string_t *object, 
                                          int64_t position,
                                          const cos_string_t *append_file, 
                                          cos_table_t *headers, 
                                          cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    int res = COSE_OK;

    /* init query_params */
    query_params = cos_table_create_if_null(options, query_params, 2);
    apr_table_add(query_params, COS_APPEND, "");
    cos_table_add_int64(query_params, COS_POSITION, position);
    
    /* init headers */
    headers = cos_table_create_if_null(options, headers, 2);
    set_content_type(append_file->data, object->data, headers);
    apr_table_add(headers, COS_EXPECT, "");

    cos_init_object_request(options, bucket, object, HTTP_POST, 
                            &req, query_params, headers, NULL, 0, &resp);
    res = cos_write_request_body_from_file(options->pool, append_file, req);

    s = cos_status_create(options->pool);
    if (res != COSE_OK) {
        cos_file_error_status_set(s, res);
        return s;
    }

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    return s;
}

cos_status_t *cos_do_append_object_from_file(const cos_request_options_t *options,
                                             const cos_string_t *bucket, 
                                             const cos_string_t *object, 
                                             int64_t position,
                                             uint64_t init_crc,
                                             const cos_string_t *append_file, 
                                             cos_table_t *headers, 
                                             cos_table_t *params,
                                             cos_progress_callback progress_callback,
                                             cos_table_t **resp_headers,
                                             cos_list_t *resp_body)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    int res = COSE_OK;

    /* init query_params */
    query_params = cos_table_create_if_null(options, params, 2);
    apr_table_add(query_params, COS_APPEND, "");
    cos_table_add_int64(query_params, COS_POSITION, position);
    
    /* init headers */
    headers = cos_table_create_if_null(options, headers, 2);
    set_content_type(append_file->data, object->data, headers);
    apr_table_add(headers, COS_EXPECT, "");

    cos_init_object_request(options, bucket, object, HTTP_POST,  &req, query_params, 
                            headers, progress_callback, init_crc, &resp);
    res = cos_write_request_body_from_file(options->pool, append_file, req);

    s = cos_status_create(options->pool);
    if (res != COSE_OK) {
        cos_file_error_status_set(s, res);
        return s;
    }

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);
    cos_fill_read_response_body(resp, resp_body);

    if (is_enable_crc(options) && has_crc_in_response(resp)) {
        cos_check_crc_consistent(req->crc64, resp->headers, s);
    }

    return s;
}

cos_status_t *cos_put_object_acl(const cos_request_options_t *options, 
                                 const cos_string_t *bucket,
                                 const cos_string_t *object, 
                                 cos_acl_e cos_acl,
                                 const cos_string_t *grant_read,
                                 const cos_string_t *grant_write,
                                 const cos_string_t *grant_full_ctrl,
                                 cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    cos_table_t *headers = NULL;
    const char *cos_acl_str = NULL;

    query_params = cos_table_create_if_null(options, query_params, 1);
    apr_table_add(query_params, COS_ACL, "");

    headers = cos_table_create_if_null(options, headers, 4);
    cos_acl_str = get_cos_acl_str(cos_acl);
    if (cos_acl_str) {
        apr_table_add(headers, COS_CANNONICALIZED_HEADER_ACL, cos_acl_str);
    }
    if (grant_read && !cos_is_null_string((cos_string_t *)grant_read)) {
        apr_table_add(headers, COS_GRANT_READ, grant_read->data);
    }
    if (grant_write && !cos_is_null_string((cos_string_t *)grant_write)) {
        apr_table_add(headers, COS_GRANT_WRITE, grant_write->data);
    }
    if (grant_full_ctrl && !cos_is_null_string((cos_string_t *)grant_full_ctrl)) {
        apr_table_add(headers, COS_GRANT_FULL_CONTROL, grant_full_ctrl->data);
    }

    cos_init_object_request(options, bucket, object, HTTP_PUT, &req, 
                            query_params, headers, NULL, 0, &resp);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    return s;    
}

cos_status_t *cos_get_object_acl(const cos_request_options_t *options, 
                                 const cos_string_t *bucket,
                                 const cos_string_t *object,
                                 cos_acl_params_t *acl_param, 
                                 cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    int res;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    cos_table_t *headers = NULL;

    query_params = cos_table_create_if_null(options, query_params, 1);
    apr_table_add(query_params, COS_ACL, "");

    headers = cos_table_create_if_null(options, headers, 0);    

    cos_init_object_request(options, bucket, object, HTTP_GET, &req, 
                            query_params, headers, NULL, 0, &resp);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);
    if (!cos_status_is_ok(s)) {
        return s;
    }

    res = cos_acl_parse_from_body(options->pool, &resp->body, acl_param);
    if (res != COSE_OK) {
        cos_xml_error_status_set(s, res);
    }

    return s;
}

cos_status_t *cos_copy_object(const cos_request_options_t *options,
                              const cos_string_t *copy_source, 
                              const cos_string_t *dest_bucket, 
                              const cos_string_t *dest_object,
                              cos_table_t *headers,
                              cos_copy_object_params_t *copy_object_param,
                              cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    int res;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;

    s = cos_status_create(options->pool);

    headers = cos_table_create_if_null(options, headers, 2);
    query_params = cos_table_create_if_null(options, query_params, 0);

    /* init headers */
    apr_table_add(headers, COS_CANNONICALIZED_HEADER_COPY_SOURCE, copy_source->data);
    set_content_type(NULL, dest_object->data, headers);

    cos_init_object_request(options, dest_bucket, dest_object, HTTP_PUT, 
                            &req, query_params, headers, NULL, 0, &resp);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);
    if (!cos_status_is_ok(s)) {
        return s;
    }

    res = cos_copy_object_parse_from_body(options->pool, &resp->body, copy_object_param);
    if (res != COSE_OK) {
        cos_xml_error_status_set(s, res);
    }

    return s;
}


#if 0
cos_status_t *cos_post_object_restore(const cos_request_options_t *options,
                                            const cos_string_t *bucket, 
                                            const cos_string_t *object,
                                            cos_object_restore_params_t *restore_params,
                                            cos_table_t *headers,
                                            cos_table_t *params,
                                            cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    cos_list_t body;
    unsigned char *md5 = NULL;
    char *buf = NULL;
    int64_t body_len;
    char *b64_value = NULL;
    int b64_buf_len = (20 + 1) * 4 / 3;
    int b64_len;

    query_params = cos_table_create_if_null(options, params, 1);
    apr_table_add(query_params, COS_RESTORE, "");

    headers = cos_table_create_if_null(options, headers, 1);

    cos_init_object_request(options, bucket, object, HTTP_POST, 
                            &req, query_params, headers, NULL, 0, &resp);

    build_object_restore_body(options->pool, restore_params, &body);

    //add Content-MD5
    body_len = cos_buf_list_len(&body);
    buf = cos_buf_list_content(options->pool, &body);
    md5 = cos_md5(options->pool, buf, (apr_size_t)body_len);
    b64_value = cos_pcalloc(options->pool, b64_buf_len);
    b64_len = cos_base64_encode(md5, 16, b64_value);
    b64_value[b64_len] = '\0';
    apr_table_addn(headers, COS_CONTENT_MD5, b64_value);
    
    cos_write_request_body_from_buffer(&body, req);

    s = cos_process_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    return s;
}



char *cos_gen_signed_url(const cos_request_options_t *options,
                         const cos_string_t *bucket, 
                         const cos_string_t *object,
                         int64_t expires, 
                         cos_http_request_t *req)
{
    cos_string_t signed_url;
    char *expires_str = NULL;
    cos_string_t expires_time;
    int res = COSE_OK;

    expires_str = apr_psprintf(options->pool, "%" APR_INT64_T_FMT, expires);
    cos_str_set(&expires_time, expires_str);
    cos_get_object_uri(options, bucket, object, req);
    res = cos_get_signed_url(options, req, &expires_time, &signed_url);
    if (res != COSE_OK) {
        return NULL;
    }
    return signed_url.data;
}

cos_status_t *cos_put_object_from_buffer_by_url(const cos_request_options_t *options,
                                                const cos_string_t *signed_url, 
                                                cos_list_t *buffer, 
                                                cos_table_t *headers,
                                                cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;

    /* init query_params */
    headers = cos_table_create_if_null(options, headers, 0);
    query_params = cos_table_create_if_null(options, query_params, 0);

    cos_init_signed_url_request(options, signed_url, HTTP_PUT, 
                                &req, query_params, headers, &resp);

    cos_write_request_body_from_buffer(buffer, req);

    s = cos_process_signed_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    if (is_enable_crc(options) && has_crc_in_response(resp)) {
        cos_check_crc_consistent(req->crc64, resp->headers, s);
    }

    return s;
}

cos_status_t *cos_put_object_from_file_by_url(const cos_request_options_t *options,
                                              const cos_string_t *signed_url, 
                                              cos_string_t *filename, 
                                              cos_table_t *headers,
                                              cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;
    int res = COSE_OK;

    s = cos_status_create(options->pool);

    headers = cos_table_create_if_null(options, headers, 0);
    query_params = cos_table_create_if_null(options, query_params, 0);

    cos_init_signed_url_request(options, signed_url, HTTP_PUT, 
                                &req, query_params, headers, &resp);
    res = cos_write_request_body_from_file(options->pool, filename, req);
    if (res != COSE_OK) {
        cos_file_error_status_set(s, res);
        return s;
    }

    s = cos_process_signed_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    if (is_enable_crc(options) && has_crc_in_response(resp)) {
        cos_check_crc_consistent(req->crc64, resp->headers, s);
    }

    return s;
}

cos_status_t *cos_get_object_to_buffer_by_url(const cos_request_options_t *options,
                                              const cos_string_t *signed_url, 
                                              cos_table_t *headers,
                                              cos_table_t *params,
                                              cos_list_t *buffer,
                                              cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;

    headers = cos_table_create_if_null(options, headers, 0);
    params = cos_table_create_if_null(options, params, 0);
    
    cos_init_signed_url_request(options, signed_url, HTTP_GET, 
                                &req, params, headers, &resp);

    s = cos_process_signed_request(options, req, resp);
    cos_fill_read_response_body(resp, buffer);
    cos_fill_read_response_header(resp, resp_headers);

    if (is_enable_crc(options) && has_crc_in_response(resp) &&  
        !has_range_or_process_in_request(req)) {
            cos_check_crc_consistent(resp->crc64, resp->headers, s);
    }

    return s;
}

cos_status_t *cos_get_object_to_file_by_url(const cos_request_options_t *options,
                                            const cos_string_t *signed_url, 
                                            cos_table_t *headers, 
                                            cos_table_t *params,
                                            cos_string_t *filename,
                                            cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    int res = COSE_OK;
    cos_string_t tmp_filename;

    s = cos_status_create(options->pool);

    headers = cos_table_create_if_null(options, headers, 0);
    params = cos_table_create_if_null(options, params, 0);

    cos_get_temporary_file_name(options->pool, filename, &tmp_filename);
 
    cos_init_signed_url_request(options, signed_url, HTTP_GET, 
                                &req, params, headers, &resp);

    res = cos_init_read_response_body_to_file(options->pool, filename, resp);
    if (res != COSE_OK) {
        cos_file_error_status_set(s, res);
        return s;
    }

    s = cos_process_signed_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    if (is_enable_crc(options) && has_crc_in_response(resp) && 
        !has_range_or_process_in_request(req)) {
            cos_check_crc_consistent(resp->crc64, resp->headers, s);
    }

    cos_temp_file_rename(s, tmp_filename.data, filename->data, options->pool);

    return s;
}


cos_status_t *cos_head_object_by_url(const cos_request_options_t *options,
                                     const cos_string_t *signed_url, 
                                     cos_table_t *headers, 
                                     cos_table_t **resp_headers)
{
    cos_status_t *s = NULL;
    cos_http_request_t *req = NULL;
    cos_http_response_t *resp = NULL;
    cos_table_t *query_params = NULL;

    headers = cos_table_create_if_null(options, headers, 0);
    query_params = cos_table_create_if_null(options, query_params, 0);
    
    cos_init_signed_url_request(options, signed_url, HTTP_HEAD, 
                                &req, query_params, headers, &resp);

    s = cos_process_signed_request(options, req, resp);
    cos_fill_read_response_header(resp, resp_headers);

    return s;
}
#endif

