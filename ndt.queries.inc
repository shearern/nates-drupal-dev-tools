<?php


$NDT_NEXT_PARM_ID=0;
function _ndt_get_next_parm_name($name)
{
    global $NDT_NEXT_PARM_ID;
    $NDT_NEXT_PARM_ID += 1;
    return sprintf("{$name}_%d", $NDT_NEXT_PARM_ID);
}

/**
 * Modify a query to join a field data table to the nodes table
 * 
 * (Node -> Field)
 * 
 * New fields should be available:
 *     $field_tab_alias.entity_type        (e.g.: node)
 *     $field_tab_alias.bundle             (Content type/Bundle)
 *     $field_tab_alias.deleted
 *     $field_tab_alias.entity_id          (e.g.: NID)
 *     $field_tab_alias.revision_id
 *     $field_tab_alias.language
 *     $field_tab_alias.delta
 *     $field_tab_alias.<field_type_values>
 * 
 * @param DrupalQuery $query:
 *     Dynamic query object
 * @param string $node_tab_alias:
 *     Alias already node table that is the left side of the join
 * @param string $field_name:
 *     Name of the field of interest
 * @param string $field_tab_alias:
 *     Alias to assign to field table (right side of join) in the query 
 * @param string $node_content_type:
 *     Specify the content_type (bundle) of the nodes.
 *     This is used, if not null, to provide an additional condition on the
 *     join to make sure we're joining to the correct node type.
 * @param string $entity_type:
 *     Specify the entity_type used to isolate the entity_id to a specific
 *     entity type.  Defaults to 'node'
 */
function ndt_join_field_data_to_node($query,
    $node_tab_alias, $field_name, $field_tab_alias,
    $node_content_type=null, $entity_type='node')
{
    $condition = array();
    $parms = array();
    
    // Join parent node to field values
    if ($entity_type == 'node')
    {
        $condition[] = "$node_tab_alias.nid = $field_tab_alias.entity_id";
        $condition[] = "$node_tab_alias.vid = $field_tab_alias.revision_id";
    }
    elseif ($entity_type == 'taxonomy_term')
    {
        $condition[] = "$node_tab_alias.tid = $field_tab_alias.entity_id";
    }
    else
        throw new Exception("Unkown entity type: $entity_type");
    
    // Additional conditions
    $del_parm = _ndt_get_next_parm_name('deleted');
    $condition[] = "$field_tab_alias.deleted = :$del_parm";
    $parms[":$del_parm"] = 0;
    
    if (!is_null($node_content_type))
    {
        $bundle_parm = _ndt_get_next_parm_name('bundle');
        $condition[] = "$field_tab_alias.bundle = :$bundle_parm";
        $parms[":$bundle_parm"] = $node_content_type;
    }
    if (!is_null($entity_type))
    {
        $et_parm = _ndt_get_next_parm_name('et');
        $condition[] = "$field_tab_alias.entity_type = :$et_parm";
        $parms[":$et_parm"] = $entity_type;
        
    }
    $condition = implode(" AND ", $condition);
    
    $right_table_name = "field_data_${field_name}";
    
    $query->leftJoin($right_table_name, $field_tab_alias, $condition, $parms);
}

/**
 * Modify a query to join a node table to a field data table
 * 
 * This is the reverse of ndt_join_field_data_to_node(), and allows us to join
 * the node table into the query based on rows in a field_data_* table based on
 * which node the fields belong to.  Note, this does not help join nodes
 * targetted by term reference or entity reference fields. 
 * 
 * (Field -> Node)
 * 
 * New fields should be available:
 *    $node_tab_alias.nid           (Node ID)
 *    $node_tab_alias.vid           (Node Version Number)
 *    $node_tab_alias.type          (Node type/Content Type/Bundle)
 *    $node_tab_alias.language
 *    $node_tab_alias.title         (Node Title)
 *    $node_tab_alias.uid
 *    $node_tab_alias.status
 *    $node_tab_alias.created
 *    $node_tab_alias.changed
 *    $node_tab_alias.comment
 *    $node_tab_alias.promote
 *    $node_tab_alias.sticky
 *    $node_tab_alias.tnid
 *    $node_tab_alias.translate
 * 
 * @param DrupalQuery $query:
 *     Dynamic query object
 * @param string $field_tab_alias:
 *     Alias already assign to field table (left side of join) in the query 
 * @param string $node_tab_alias:
 *     Alias of node table that is the right side of the join
 * @param string $node_content_type:
 *     Specify the content_type (bundle) of the nodes.
 *     This is used, if not null, to provide an additional condition on the
 *     join to make sure we're joining to the correct node type.
 */
function ndt_join_node_to_field_data($query, $field_tab_alias, $node_tab_alias,
    $node_content_type=null)
{
    $condition = array();
    $parms = array();
    $condition[] = "$field_tab_alias.entity_id = $node_tab_alias.nid";
    $condition[] = "$field_tab_alias.revision_id = $node_tab_alias.vid";
    if (!is_null($node_content_type))
    {
        $bundle_parm = _ndt_get_next_parm_name('bundle');
        $condition[] = "$node_tab_alias.type = :$bundle_parm";
        $parms[":$bundle_parm"] = $node_content_type;
    }
    $condition = implode(" AND ", $condition);
    
    $query->leftJoin('node', $node_tab_alias, $condition, $parms);
}

/**
 * Modify a query to join a taxonomy term referenced by a term reference field
 * 
 * If you already have a field_data table in the query that is for a term
 * reference field, then join in the taxonomy_term_data table to bring in
 * the refereced term.
 * 
 * (Term Ref Field -> Term)
 * 
 * New fields should be available:
 *     $term_tab_alias.tid            (Term ID)
 *     $term_tab_alias.vid            (Vocab ID)
 *     $term_tab_alias.name           (Term)
 *     $term_tab_alias.description    (Body)
 *     $term_tab_alias.format         (Body Format)
 *     $term_tab_alias.weight
 * 
 * @param DrupalQuery $query:
 *     Dynamic query object
 * @param string $field_name:
 *     Field name of the term reference field 
 * @param string $field_tab_alias:
 *     Alias already assign to field table (left side of join) in the query 
 * @param string $term_tab_alias:
 *     Alias to assign to term table (right side of join) in the query
 * @param string/int $vocab:
 *     Optional vocab to limit terms to.  Not needed. 
 */
function ndt_join_term_to_termref_field($query, $field_name, $field_tab_alias,
    $term_tab_alias, $vocab=null)
{
    if (!is_null($vocab))
        if (is_int($vocab))
            $vocab = ndt_find_vocab($vocab);

    $condition = array();
    $parms = array();
    $condition[] = "$field_tab_alias.${field_name}_tid = $term_tab_alias.tid";
    if (!is_null($vocab))
    {
        $vocab_parm = _ndt_get_next_parm_name('vocab');
        $condition[] = "$term_tab_alias.vid = :$vocab_parm";
        $parms[":$vocab_parm"] = $vocab;
    }
    $condition = implode(" AND ", $condition);
    
    $query->leftJoin('taxonomy_term_data', $term_tab_alias, $condition, $parms);
}

/**
 * Modify a query to join the term_data table to a node table through a
 * term reference field.
 * 
 * This is a proxy to call in sequence: 
 *     1) ndt_join_field_data_to_node() 
 *     2) ndt_join_term_to_termref_field()
 *     
 * @param string $node_tab_alias:
 *     Alias already node table that is the left side of the join
 * @param string $field_name:
 *     Name of the field of interest
 * @param string $field_tab_alias:
 *     Alias to assign to field table (right side of join) in the query 
 * @param string $term_tab_alias:
 *     Alias to assign to term table (right side of join) in the query
 * @param string $entity_type:
 *     Specify the entity_type used to isolate the entity_id to a specific
 *     entity type.  Defaults to 'node'
 */
function join_term_through_termref_field($query, $node_tab_alias,
    $field_name, $field_tab_alias, $term_tab_alias, $entity_type='node')
{ 
    ndt_join_field_data_to_node($query, $node_tab_alias,
        $field_name, $field_tab_alias, null, $entity_type);
    ndt_join_term_to_termref_field($query, $field_name, $field_tab_alias,
        $term_tab_alias);
}

/**
 * Modify a query to join term reference fields to the taxonomy_term_data table
 * 
 * Starting from the table that contains all of the taxonomy term data, this
 * joins all references to that term from a given term reference field data
 * table.  This is the reverse of ndt_join_node_to_field_data()
 * 
 * (Term <- Term Ref Field)
 * 
 * New fields should be available:
 *     $field_tab_alias.entity_type
 *     $field_tab_alias.bundle
 *     $field_tab_alias.deleted
 *     $field_tab_alias.entity_id
 *     $field_tab_alias.revision_id
 *     $field_tab_alias.language
 *     $field_tab_alias.delta
 *     $field_tab_alias.${field_name}_tid
 *     
 * @param string $term_tab_alias:
 *     Alias already assign to term table (left side of join) in the query
 * @param string $field_name:
 *     Field name of the term reference field 
 * @param string $field_tab_alias:
 *     Alias to assign to field table (left side of join) in the query 
 */
function ndt_join_termref_fields_to_term($query, $term_tab_alias,
    $field_name, $field_tab_alias)
{    
    $condition = array();
    $parms = array();
    $condition[] = "$term_tab_alias.tid = $field_tab_alias.${field_name}_tid";
    $condition = implode(" AND ", $condition);
    
    $del_parm = _ndt_get_next_parm_name('deleted');
    $condition[] = "$field_tab_alias.deleted = :$del_parm";
    $parms[":$del_parm"] = 0;
    
    $right_table_name = "field_data_${field_name}";
    
    $query->leftJoin($right_table_name, $term_tab_alias, $condition, $parms);
}

/**
 * Modify a query to join a node referenced by an entity reference field
 * 
 * If you already have a field_data table in the query that is for an entity
 * reference field, then join in the node table using the node reference value
 * 
 * (Entity Ref Field -> Node)
 * 
 * New fields should be available:
 *     $field_tab_alias.entity_type        (e.g.: node)
 *     $field_tab_alias.bundle             (Content type/Bundle)
 *     $field_tab_alias.deleted
 *     $field_tab_alias.entity_id          (e.g.: NID)
 *     $field_tab_alias.revision_id
 *     $field_tab_alias.language
 *     $field_tab_alias.delta
 *     $field_tab_alias.<field_type_values>
 * 
 * @param DrupalQuery $query:
 *     Dynamic query object
 * @param string $field_name:
 *     Field name of the entity reference field 
 * @param string $field_tab_alias:
 *     Alias already assigned to field table (left side of join) in the query 
 * @param string $node_tab_alias:
 *     Alias to assign to node table (right side of join) in the query
 * @param string $node_content_type:
 *     Optionally limit the node type of the reference node
 */
function ndt_join_node_to_entityref_field($query, $field_name, $field_tab_alias,
    $node_tab_alias, $node_content_type=null)
{
    $condition = array();
    $parms = array();
    $condition[] = "$field_tab_alias.${field_name}_target_id = $node_tab_alias.nid";
    if (!is_null($node_content_type))
    {
        $type_parm = _ndt_get_next_parm_name('type');
        $condition[] = "$node_tab_alias.type = :$type_parm";
        $parms[":$type_parm"] = $node_content_type;
    }
    $condition = implode(" AND ", $condition);
    
    $query->leftJoin('node', $node_tab_alias, $condition, $parms);
}


/**
 * Modify a query to join the node table to a node table through an
 * entity reference field.
 * 
 * (Nonde -> Entity Ref Field -> Node)
 * 
 * This is a proxy to call in sequence: 
 *     1) ndt_join_field_data_to_node() 
 *     2) ndt_join_node_to_entityref_field()
 *     
 * @param string $node_tab_alias:
 *     Alias already node table that is the left side of the join
 * @param string $field_name:
 *     Name of the field of interest
 * @param string $field_tab_alias:
 *     Alias to assign to field table (right side of join) in the query 
 * @param string $ref_node_tab_alias:
 *     Alias to assign to node table (right side of join) in the query
 * @param string $entity_type:
 *     Specify the entity_type used to isolate the entity_id to a specific
 *     entity type.  Defaults to 'node'
 * @param string $node_content_type:
 *     Optionally limit the node type of the reference node
 */
function join_node_through_entityref_field($query, $node_tab_alias,
    $field_name, $field_tab_alias, $ref_node_tab_alias, $entity_type='node',
    $node_content_type=null)
{ 
    ndt_join_field_data_to_node($query, $node_tab_alias,
        $field_name, $field_tab_alias, null, $entity_type);
    ndt_join_node_to_entityref_field($query, $field_name, $field_tab_alias,
        $ref_node_tab_alias, $node_content_type);
}


/**
 * Modify a query to join in entity reference field values that relate to nodes
 * 
 * If you already have a node table referenced in the query, use this to join
 * in an entity reference field data table based on which field values are
 * referencing nodes already selected by the query.
 * 
 * (Node <- Entity Ref Field)
 * 
 * @param string $node_tab_alias:
 *     Alias already assigned to node table (left side of join) in the query
 * @param string $field_name:
 *     Field name of the entity reference field 
 * @param string $field_tab_alias:
 *     Alias to assigned to field table (right side of join) in the query 
 */
function ndt_join_entityref_field_to_node($query, $node_tab_alias, $field_name,
    $field_tab_alias)
{
    $condition = array();
    $parms = array();
    $condition[] = "$node_tab_alias.nid = $field_tab_alias.${field_name}_target_id";
    
    $del_parm = _ndt_get_next_parm_name('deleted');
    $condition[] = "$field_tab_alias.deleted = :$del_parm";
    $parms[":$del_parm"] = 0;
    
    $condition = implode(" AND ", $condition);
    
    $right_table_name = "field_data_${field_name}";
    
    $query->leftJoin($right_table_name, $field_tab_alias, $condition, $parms);
}


/**
 * Modify a query to join in file_data referenced by a file field
 * 
 * If you already have a field_data table in the query that is for a file
 * reference field, then join in the file_managed table using the field value
 * 
 * (File Field -> Managed File)
 * 
 * New fields should be available:
 *     $file_tab_alias.fid        (File ID)
 *     $file_tab_alias.uid        (User ID)
 *     $file_tab_alias.filename   (Name of the file)
 *     $file_tab_alias.uri        (URI to the file)
 *     $file_tab_alias.filemime   (Time file was modified)
 *     $file_tab_alias.filesize   (Size of file)
 *     $file_tab_alias.status     (Normally 1 ?)
 *     $file_tab_alias.timestamp  
 * 
 * @param DrupalQuery $query:
 *     Dynamic query object
 * @param string $field_name:
 *     Field name of the file reference field 
 * @param string $field_tab_alias:
 *     Alias already assigned to field table (left side of join) in the query 
 * @param string $file_tab_alias:
 *     Alias to assign to file table (right side of join) in the query
 */
function ndt_join_file_to_field($query, $field_name, $field_tab_alias,
    $file_tab_alias)
{
    $condition = array();
    $parms = array();
    $condition[] = "$field_tab_alias.${field_name}_fid = $file_tab_alias.fid";
    $condition = implode(" AND ", $condition);
    
    $query->leftJoin('file_managed', $file_tab_alias, $condition, $parms);
}

function ndt_run_query_as_count($query)
{
    return $query->countQuery()->execute()->fetchField();
}
