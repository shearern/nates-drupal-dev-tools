<?php

/**
 * List all of the item in a SystemQueue
 * 
 * @param string $queue_name:
 *     Queue name
 * @param boolean $in_order:
 *     If True, then will return items in order they were added
 */
function ndt_list_jobs_in_drupal_queue($queue_name, $in_order=False)
{
    $rtn = array();
    
    $query = db_select('queue', 'q');
    $query->condition('q.name', $queue_name);
    if ($in_order)
        $query->orderby('q.created', 'ASC');
    $query->addField('q', 'data');
    foreach ($query->execute() as $row)
        $rtn[] = unserialize($row->data);
    
    return $rtn;
}