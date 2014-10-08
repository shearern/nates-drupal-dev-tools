<?php

/**
 * Store a variable to the DB for retrieval by ndt_get()
 * 
 * @param string $domain:
 *     Domain is a grouping of variables namess.
 * @param string $var_name:
 *     Name of variable to set (will delete existing if exists)
 * @param mixed $value:
 *     Value to save (will be serialized)
 * @param int $expire_seconds:
 *     How many minutes to hold on the value (null for forever)
 */
function ndt_set($domain, $var_name, $value, $expire_mins=null)
{
    ndt_delete($domain, $var_name);
    
    $fields = array(
        'domain' => $domain,
        'var_name' => $var_name,
        'var_value' => serialize($value),
        );
    if (!is_null($expire_mins))
        $fields['auto_expires_at'] = time() + (60 * $expire_mins);
    
    db_insert('ndt_variables')->fields($fields)->execute();
}

function ndt_get($domain, $var_name, $default=null, $use_cache=FALSE)
{
    // Retrieve from cache if available
    $cache = &drupal_static(__FUNCTION__);
    if ($use_cache)
        if (isset($cache[$domain]) && isset($cache[$domain][$var_name]))
            return $cache[$domain][$var_name];
    
    // Retrieve value from DB
    $value = $default;
    
    $query = db_select('ndt_variables', 'v');
    $query->condition('domain', $domain);
    $query->condition('var_name', $var_name);
    $query->addField('v', 'var_value');
    foreach ($query->execute() as $row)
        $value = unserialize($row->var_value);
    
    // Cache retrieved value
    if ($use_cache)
    {
        if (!isset($cache[$domain]))
            $cache[$domain] = array();
        $cache[$domain][$var_name] = $value;
    }
    
    return $value;
}

function ndt_delete($domain, $var_name)
{
    db_delete('ndt_variables')
        ->condition('domain', $domain)
        ->condition('var_name', $var_name)
        ->execute();
}

function ndt_list_vars($domain)
{
    $rtn = array();
    $query = db_select('ndt_variables');
    $query->condition('domain', $domain);
    $query->addField('v', 'var_name');
    $query->distinct();
    foreach ($query->execute() as $row)
        $rtn[] = $row->var_name;
    return $rtn;
}

function _ndt_auto_delete_variables()
{
    db_delete('ndt_variables')
        ->isNotNull('auto_expires_at')
        ->condition('auto_expires_at', time(), '<')
        ->execute();
}