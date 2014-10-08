<?php

/**
 * Create a new node
 */
function ndt_create_new_node($node_type, $title=null, $and_save=False)
{
    $node = new stdClass();
    $node->type = $node_type;
    $node->status = 1;    # Published
    
    if (!is_null($title))
        $node->title = $title;
    
    if ($and_save)
        node_save($node);
    
    return $node;
}
