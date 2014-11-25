<?php


// -- Field Discovery (move to ndt) --------------------------------------

function ndt_list_node_fields($node)
{
    $rtn = array();

    $fields = field_info_instances('node', $node->type);

    foreach ($fields as $name => $attribs)
    {
        $info = array(
            'name' => $attribs['field_name'],
            'label' => $attribs['label'],
            );

        $field_info = field_info_field($name);
        //dpm($field_info, $name);
        $info['module'] = $field_info['module'];
        $info['type'] = $field_info['type'];
        $info['columns'] = array_keys($field_info['columns']);

        // // Map to functions
        // $info['get_func'] = null;
        // $info['get_single_func'] = null;
        // $info['add_func'] = null;
        // $info['clear_func'] = null;
        // $info['has_value_func'] = null;
        // if ($info['module'] == 'text' and $info['type'] == 'text')
        // {
        //     $info['get_func'] = "ndt_get_str_values(\$entity, '$name')";
        //     $info['get_single_func'] = "ndt_get_str_value(\$entity, '$name', \$default=null)";
        //     $info['add_func'] = "ndt_add_str_value(\$entity, '$name', \$value)";
        //     $info['count_func'] = "ndt_count_str_values(\$entity, '$name')";
        //     $info['clear_func'] = null;
        //     $info['has_value_func'] = "ndt_has_str_values(\$entity, '$name')";
        // }

        $rtn[] = (object) $info;
    }

    return $rtn;
}

function ndt_repr($var)
{
    return var_export($var, True);
}


// -- Generic Field Manipulation ----------------------------------------------

/**
 * Add a value to a Drupal field
 *
 * @param Entity $entity
 *         Entity to examine
 * @param string $field_name
 *         Name of the field
 * @param string $value_struct
 *         Array with value to store formatted for this field
 * @param string $lang
 *         Language Code
 */
function ndt_add_field_value(&$entity, $field_name, $value_struct, $lang=null)
{
    // Create field container
    $field = array();
    if (isset($entity->$field_name))
        $field = $entity->$field_name;

    // Create language container
    if (is_null($lang))
        $lang = LANGUAGE_NONE;
    if (!isset($field[$lang]))
        $field[$lang] = array();

    // Create delta
    $delta = count($field[$lang]);
    while (isset($field[$lang][$delta]))
        $delta += 1;
    $field[$lang][$delta] = $value_struct;

    // Save new value
    $entity->$field_name = $field;
}

/**
 * Set a single value to a Drupal field
 *
 * @param Entity $entity
 *         Entity to examine
 * @param string $field_name
 *         Name of the field
 * @param string $value_struct
 *         Array with value to store formatted for this field
 * @param string $lang
 *         Language Code
 */
function ndt_set_field_value(&$entity, $field_name, $value_struct, $lang=null)
{
    // Create field container
    $field = array();

    // Create language container
    $field[$lang] = array();

    // Create delta
    $field[$lang] = array(0 => $value_struct);

    // Save new value
    $entity->$field_name = $field;
}

/**
 * Clear all values from a field
 *
 * @param Entity $entity
 *         Entity to examine
 * @param string $field_name
 *         Name of the field
 */
function ndt_clear_field_values(&$entity, $field_name)
{
    if (isset($entity->$field_name))
    {
        $field =& $entity->$field_name;
        foreach ($field as $lang => $values)
            $field[$lang] = array();
    }
}

/**
 * Examine a Drupal field to extract values
 *
 * @param Entity $entity
 *     Entity to examine
 * @param string $field_name
 *     Name of the field to extract value for (with field_ prefix)
 * @param string $value_key
 *     Array key that stores the value we want to extract
 * @return array
 *     Array of extracted values
 */
function ndt_extract_field_values($entity, $field_name, $value_key)
{
    $rtn = array();

    if (isset($entity->$field_name))
    {
        $field = $entity->$field_name;
        foreach ($field as $lang_code => $values)
            foreach ($values as $delta => $value)
                if (isset($value[$value_key]))
                    $rtn[$delta] = $value[$value_key];
    }

    return $rtn;
}

/**
 * Examine a Drupal field to extract values
 *
 * @param Entity $entity
 *     Entity to examine
 * @param string $field_name
 *     Name of the field to extract value for (with field_ prefix)
 * @param string $value_key
 *     Array key that stores the value we want to extract
 * @return array
 *     Array of extracted values
 */
function ndt_extract_field_values_without_entity($entity, $field_name, $value_key)
{
    // Legacy for when we were using entity wrappers
    return ndt_extract_field_values($entity, $field_name, $value_key);
}


/**
 * Examine a Drupal field to extract a single value
 *
 * @param Entity $entity
 *         Entity to examine
 * @param string $field_name
 *         Name of the field to extract value for (with field_ prefix)
 * @param string $value_key
 *         Array key that stores the value we want to extract
 * @param mixed $default
 *         Default value to return if nothing is stored for this field
 * @return mixed
 *         Value stored for this field
 */
function ndt_extract_field_value($entity, $field_name, $value_key, $default=null)
{
    $values = ndt_extract_field_values($entity, $field_name, $value_key);
    if (count($values) == 0)
        return $default;
    elseif (count($values) == 1)
        return reset($values);
    else
    {
        $cnt = count($values);
        throw new Exception(
            "Expected exacly 1 value for field $field_name, but got $cnt");
    }
}


/**
 * Examine a Drupal field to extract a single value
 *
 * @param Entity $entity
 *         Entity to examine
 * @param string $field_name
 *         Name of the field to extract value for (with field_ prefix)
 * @param string $value_key
 *         Array key that stores the value we want to extract
 * @param mixed $default
 *         Default value to return if nothing is stored for this field
 * @return mixed
 *         Value stored for this field
 */
function ndt_extract_field_value_without_entity($entity, $field_name,
    $value_key, $default=null)
{
    return ndt_extract_field_value($entity, $field_name, $value_key, $default);
}

/**
 * Examine a Drupal field to see if any values are stored
 *
 * @param Entity $entity
 *         Entity to examine
 * @param string $field_name
 *         Name of the field to extract value for (with field_ prefix)
     * @param string $value_key
 *         Array key that stores the value we want to extract
 * @return mixed
 *         Value stored for this field
 */
function ndt_field_has_values($entity, $field_name, $value_key)
{
    $values = ndt_extract_field_values($entity, $field_name, $value_key);
    return count($values) > 0;
}

// -- Integer fields -----------------------------------------------------------

/**
 * Get string values
 *
 * @param Entity $entity
 *         Entity with term reference
 * @param string $field_name
 *         Field name to extract value from
 * @return array
 *         Array of string values
 */
function ndt_get_int_values($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_extract_field_values($entity, $field_name, 'value') as $val)
        $rtn[] = intval($val);
    return $rtn;
}
function ndt_get_int_value($entity, $field_name, $default=null)
{
    return intval(ndt_extract_field_value($entity, $field_name, 'value', $default));
}
function ndt_has_int_values($entity, $field_name)
{
    return count(ndt_get_int_values($entity, $field_name)) > 0;
}
function ndt_count_int_values($entity, $field_name)
{
    $cnt = 0;
    foreach (ndt_get_int_values($entity, $field_name) as $value)
        if (strlen(trim($value)) > 0)
            $cnt += 1;
    return $cnt;
}
function ndt_add_int_value($entity, $field_name, $value)
{
    /*
    $entity = entity_metadata_wrapper('node', $entity);
    $field = $entity->$field_name;
    $field[] = $value;
    
    drupal_set_message(sprintf("Check that adding '%s' to '%s' on %d worked",
        $field_name, $value, $entity->nid->value()));
    */

    ndt_add_field_value($entity, $field_name, array('value' => $value));
}
function ndt_set_int_value($entity, $field_name)
{
    return ndt_set_field_value($entity, $field_name, array('value' => $value));
}


// -- String fields -----------------------------------------------------------

/**
 * Get string values
 *
 * @param Entity $entity
 *         Entity with term reference
 * @param string $field_name
 *         Field name to extract value from
 * @return array
 *         Array of string values
 */
function ndt_get_str_values($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_extract_field_values($entity, $field_name, 'value') as $val)
        $rtn[] = $val;
    return $rtn;
}
function ndt_get_str_value($entity, $field_name, $default=null)
{
    return ndt_extract_field_value($entity, $field_name, 'value', $default);
}
function ndt_has_str_values($entity, $field_name)
{
    return count(ndt_get_str_values($entity, $field_name)) > 0;
}
function ndt_count_str_values($entity, $field_name)
{
    $cnt = 0;
    foreach (ndt_get_str_values($entity, $field_name) as $value)
        if (strlen(trim($value)) > 0)
            $cnt += 1;
    return $cnt;
}
function ndt_add_str_value($entity, $field_name, $value)
{
    /*
    $entity = entity_metadata_wrapper('node', $entity);
    $field = $entity->$field_name;
    $field[] = $value;
    
    drupal_set_message(sprintf("Check that adding '%s' to '%s' on %d worked",
        $field_name, $value, $entity->nid->value()));
    */

    ndt_add_field_value($entity, $field_name, array('value' => $value));
}
function ndt_set_str_value($entity, $field_name)
{
    return ndt_set_field_value($entity, $field_name, array('value' => $value));
}


// -- Term Reference fields ---------------------------------------------------

/**
 * Get term reference values
 * @param Entity $entity
 *         Entity with term reference
 * @param string $field_name
 *         Field name to extract value from
 * @return array
 *         Array of term IDs
 */
function ndt_get_termref_values($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_extract_field_values($entity, $field_name, 'tid') as $tid)
        $rtn[] = $tid;
    return $rtn;
}
function ndt_get_termref_value($entity, $field_name, $default=null)
{
    return ndt_extract_field_value($entity, $field_name, 'tid', $default);
}
function ndt_has_termref_values($entity, $field_name)
{
    return count(ndt_get_termref_values($entity, $field_name)) > 0;
}
function ndt_add_termref_value($entity, $field_name, $tid)
{
    ndt_add_field_value($entity, $field_name, array('tid' => $tid));
}
function ndt_set_termref_value($entity, $field_name, $tid)
{
    ndt_set_field_value($entity, $field_name, array('tid' => $tid));
}
function ndt_get_termref_tids($entity, $field_name)
{
    return ndt_get_termref_values($entity, $field_name);
}
function ndt_get_termref_tid($entity, $field_name, $default=null)
{
    return ndt_get_termref_value($entity, $field_name);
}
function ndt_get_termref_terms($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_get_termref_values($entity, $field_name) as $tid)
        $rtn[] = taxonomy_term_load($tid)->name;
    return $rtn;
}
function ndt_get_termref_term($entity, $field_name, $default=null)
{
    $tid = ndt_get_termref_tid($entity, $field_name);
    if (is_null($tid))
        return null;
    return taxonomy_term_load($tid)->name;
}


// -- Entity Reference fields -------------------------------------------------

/**
 * Get entity reference values
 * @param Entity $entity
 *         Entity with term reference
 * @param string $field_name
 *         Field name to extract value from
 * @return array
 *         Array of term IDs
 */
function ndt_get_entityref_values($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_extract_field_values($entity, $field_name, 'target_id') as $eid)
        $rtn[] = $eid;
    return $rtn;
}
function ndt_get_entityref_nodes($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_get_entityref_values($entity, $field_name) as $eid)
        $rtn[] = node_load($eid);
    return $rtn;
}
function ndt_get_entityref_node($entity, $field_name)
{
    foreach (ndt_get_entityref_values($entity, $field_name) as $eid)
        return node_load($eid);
}
function ndt_has_entityref_values($entity, $field_name)
{
    return count(ndt_get_entityref_values($entity, $field_name)) > 0;
}
function ndt_add_entityref_value($entity, $field_name, $eid)
{
    /*
    $entity = entity_metadata_wrapper('node', $entity);
    $field = $entity->$field_name;
    $field[] = $eid;
    
    drupal_set_message(sprintf("Check that adding '%s' to '%s' on %d worked",
        $field_name, $eid, $entity->nid->value()));
    */

    ndt_add_field_value($entity, $field_name, array('target_id' => $eid));
}
function ndt_set_entityref_value($entity, $field_name, $eid)
{
    /*
    $entity = entity_metadata_wrapper('node', $entity);
    $field = $entity->$field_name;
    $field[] = $eid;
    
    drupal_set_message(sprintf("Check that adding '%s' to '%s' on %d worked",
        $field_name, $eid, $entity->nid->value()));
    */

    ndt_set_field_value($entity, $field_name, array('target_id' => $eid));
}


// -- File Fields -------------------------------------------------------------

/**
 * Get File ID values
 *
 * @param Entity $entity
 *         Entity with term reference
 * @param string $field_name
 *         Field name to extract value from
 * @return array
 *         Array of string values
 */
function ndt_get_file_fid_values($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_extract_field_values($entity, $field_name, 'fid') as $val)
        $rtn[] = $val;
    return $rtn;
}

/**
 * Get File URI values
 *
 * @param Entity $entity
 *         Entity with term reference
 * @param string $field_name
 *         Field name to extract value from
 * @return array
 *         Array of string values
 */
function ndt_get_file_uri_values($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_extract_field_values($entity, $field_name, 'uri') as $val)
        $rtn[] = $val;
    return $rtn;
}

/**
 * Get File Paths (on disk paths)
 *
 * @param Entity $entity
 *         Entity with term reference
 * @param string $field_name
 *         Field name to extract value from
 * @return array
 *         Array of string values
 */
function ndt_get_file_field_paths($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_extract_field_values($entity, $field_name, 'uri') as $val)
        $rtn[] = drupal_realpath($val);
    return $rtn;
}


// -- Date fields -------------------------------------------------------------

/**
 * Get date values
 *
 * @param Entity $entity
 *         Entity with term reference
 * @param string $field_name
 *         Field name to extract value from
 * @return array
 *         Array of string values
 */
function ndt_get_date_str_values($entity, $field_name)
{
    $rtn = array();
    foreach (ndt_extract_field_values($entity, $field_name, 'value') as $val)
        $rtn[] = $val;
    return $rtn;
}
function ndt_get_date_str_value($entity, $field_name, $default=null)
{
    return ndt_extract_field_value($entity, $field_name, 'value', $default);
}
function ndt_has_date_values($entity, $field_name)
{
    return count(ndt_get_str_values($entity, $field_name)) > 0;
}
function ndt_count_date_values($entity, $field_name)
{
    $cnt = 0;
    foreach (ndt_get_str_values($entity, $field_name) as $value)
        if (strlen(trim($value)) > 0)
            $cnt += 1;
    return $cnt;
}
function ndt_add_date_str_value($entity, $field_name, $value)
{
    /*
    $entity = entity_metadata_wrapper('node', $entity);
    $field = $entity->$field_name;
    $field[] = $value;
    
    drupal_set_message(sprintf("Check that adding '%s' to '%s' on %d worked",
        $field_name, $value, $entity->nid->value()));
    */

    ndt_add_field_value($entity, $field_name, array('value' => $value));
}
function ndt_set_date_str_value($entity, $field_name, $value)
{
    //TODO: This doesn't work.  Use DateObject
    return ndt_set_field_value($entity, $field_name, array('timestamp' => $value));  // Test
}
