<?php

// -- Taxonomy Terms ----------------------------------------------------------

/**
 * Determine a vocab vid
 */
function ndt_find_vocab($name, $required=True)
{
    $cached = &drupal_static(__FUNCTION__);
    
    if (!isset($cached[$name]))
    {
        $query = db_select('taxonomy_vocabulary', 'v');
        $query->fields('v', array('vid'));
        $query->condition('v.machine_name', $name);
        $vid = $query->execute()->fetchField();
    
        if ($required && empty($vid))
            throw new Exception("Failed to find vocab '$name'");
        else
            $cached[$name] = $vid;
    }
    
    return $cached[$name];
}

/**
 * Get all term IDs within a vocabulary
 * 
 * @param mixed $vocab
 *      Vocabulary to search.
 *      If numeric, then should be vid.
 *      Else, vocabulary name
 *  @return array
 *      List of term IDs
 */
function ndt_list_vocab_tids($vocab)
{
    $rtn = array();
    
    // Resolve vocab reference to vid
    if (!is_numeric($vocab))
        $vocab = ndt_find_vocab($vocab);
    $vocab = intval($vocab);
    
    // List term IDs
    $query = db_select('taxonomy_term_data', 't');
    $query->fields('t', array('tid'));
    $query->condition('t.vid', $vocab);
    foreach ($query->execute() as $row)
        $rtn[] = $row->tid;
    
    return $rtn;
}

/**
 * Get all terms within a vocabulary
 * 
 * @param mixed $vocab
 *      Vocabulary to search.
 *      If numeric, then should be vid.
 *      Else, vocabulary name
 *  @return array
 *      List of term objects
 */
function ndt_list_vocab_terms($vocab)
{
    $rtn = array();
    foreach (ndt_list_vocab_tids($vocab) as $tid)
        $rtn[] = taxonomy_term_load($tid);
    return $rtn;
}

/**
 * Find a term TID with the given name in the given format
 * 
 * @param mixed $vocab
 *         Vocabulary to search.
 *         If numeric, then should be vid.
 *         Else, vocabulary name
 * @param string $name
 *         Term name to search for
 * @param boolean $return_tid
 *         If false, return term instead of TID
 */
function ndt_find_term($vocab, $name, $return_tid=True)
{
    // Resolve vocab reference to vid
    if (!is_numeric($vocab))
        $vocab = ndt_find_vocab($vocab);
    $vocab = intval($vocab);
    
    // Fetch Term TID
    $tid = null;
    $query = db_select('taxonomy_term_data', 't');
    $query->fields('t', array('tid'));
    $query->condition('t.vid', $vocab);
    $query->condition('t.name', $name);
    foreach ($query->execute() as $term)
        $tid = $term->tid;
    
    // Convert TID to term
    if (!$return_tid)
        $tid = taxonomy_term_load($tid);
    
    return $tid;
}

/**
 * Load a term by it's TID
 */
function ndt_load_term($tid)
{
    return taxonomy_term_load($tid);
}

/**
 * Find a term TID with the given name in the given format
 *
 * @param mixed $vocab
 *         Vocabulary to search.
 *         If numeric, then should be vid.
 *         Else, vocabulary name
 * @param string $name
 *         Term name to create
 * @return int $tid
 *         TID of crated term
 */
function ndt_create_term($vocab, $name)
{
    // Resolve vocab reference to vid
    if (!is_numeric($vocab))
        $vocab = ndt_find_vocab($vocab);
    $vocab = intval($vocab);
    
    // Create term
    $term = new stdClass();
    $term->name = $name;
    $term->vid = $vocab;
    taxonomy_term_save($term);
    
    // Return TID
    return $term->tid;
}

function ndt_wrap_term($term)
{
    return entity_metadata_wrapper('taxonomy_term', $term);
}

