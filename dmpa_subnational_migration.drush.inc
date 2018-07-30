<?php
use \Drupal\taxonomy\Entity\Term;
/**
 * Implements hook_drush_command().
 */
function dmpa_subnational_migration_drush_command() {
  $items = array();

  $items['dmpa_subnational_migration_level_one'] = array(
    'description' => 'Migrates sub-national level one',
    'aliases' => ['lvl_one'],
  );

  return $items;
}

function drush_dmpa_subnational_migration_level_one(){
  drush_print('Level one');
  $module_path = drupal_get_path('module','dmpa_subnational_migration');
  $file = fopen($module_path.'/org_units.csv',"r");

  $r = 0; // row counter with 0 being first row
  while(! feof($file)){
    $row = fgetcsv($file);

    if ($r == 0){
      $r++;
      continue;
    }

    if (count(checkTermExists($row[1])) == 0){
      //print('create me');
      createTerm($row, 1);

      if (!empty($row[2])){
        $tid = checkTermExists($row[1])[0]->tid;
        createTerm($row, 2, $tid);
      }
    }
    else{
      if (!empty($row[2])){
        //print('col: '.$row[2]."\n");
        $tid = checkTermExists($row[1])[0]->tid;
        createTerm($row, 2, $tid);
      }
    }
  }
  fclose($file);
}

function getDatabaseConnection($key='default'){
  \Drupal\Core\Database\Database::setActiveConnection($key);
  $db = \Drupal\Core\Database\Database::getConnection();

  return $db;
}

function createTerm($row, $column, $tid = NULL, $vid = 'subnational'){
  $countryId = checkTermExists($row[0])[0]->tid;
  $parameters = [
    'name' => $row[$column],
    'vid' => $vid,
    'description' => $row[$column],
    'field_ensi' => ['target_id' => $countryId]
  ];

  if ($column == 2){
    print("\nset parent to tid: ".$tid);
    $parameters['parent'] = ['target_id' => $tid];
  }
  Term::create($parameters)->save();
}

function checkTermExists($name){
  $db = getDatabaseConnection();
  $query = $db->select('taxonomy_term_field_data', 'fd')
    ->fields('fd', ['tid', 'name'])->condition('name', $name);
  return $query->execute()->fetchAll();
}