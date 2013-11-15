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

/**
 * Convert URIs into real paths (best guess :-) )
 */
function ndt_get_real_path($uri)
{
    $scheme = file_uri_scheme($uri);
    if ($scheme == 'public')
    {
        if ($wrapper = file_stream_wrapper_get_instance_by_uri('public://'))
        {
            $root = $wrapper->realpath();
            $target = file_uri_target($uri);
            return "$root/$target";
        }
        else
            throw new Exception("Could not find public:// root");
    }
    elseif ($scheme == 'private')
    {
        if ($wrapper = file_stream_wrapper_get_instance_by_uri('private://'))
        {
            $root = $wrapper->realpath();
            $target = file_uri_target($uri);
            return "$root/$target";
        }
        else
            throw new Exception("Could not find private:// root");
    }
    else
    {
        throw new Exception("Don't know how to handle scheme $scheme");
    }
}


/**
 * List all files in a directory recorsivly
 * 
 * @param string $search_path:
 *      Parent directory to search
 * @param boolean $include_dirs:
 *      Whether to include directory entries
 * @param boolean $include_files:
 *      Whether to include file entries
 * @return array:
 *     Paths under the parent.
 *        Files: '/dir/filename'
 *        Dirs:  '/dir/dirname/'
 */
function ndt_find($search_path, $include_dirs=False, $include_files=True)
{
    $rtn = array();
    
    foreach (scandir($search_path) as $filename)
    {
        if ($filename != '.' and $filename != '..')
        {
            $full_path = "$search_path/$filename";
            
            if (is_file($full_path) && $include_files)
                $rtn[] = $filename;
            
            elseif (is_dir($full_path))
            {
                // Include entry
                if ($include_dirs)
                    $rtn[] = "$filename/";
                
                // Decend into directory
                foreach (ndt_find($full_path, $include_dirs, $include_files) as $child)
                    $rtn[] = "$filename/$child";
            }
        }
    }
    
    return $rtn;
}
