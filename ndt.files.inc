<?php

/**
 * Given a file field, determine the URI to the directory
 */
function ndt_get_file_field_dir_uri($field_name, $content_type)
{
    $field_info = field_info_field($field_name);
    if (empty($field_info))
    {
        drupal_set_message("Invalid field name: '$field_name'",
            'error');
        return;
    }
    
    $instance_info = field_info_instance('node', $field_name, $content_type);
    if (empty($instance_info))
    {
        drupal_set_message("Invalid field instance: '$content_type.$field_name'",
            'error');
        return;
    }
    
    return sprintf("%s://%s",
        $field_info['settings']['uri_scheme'],
        $instance_info['settings']['file_directory']);
}