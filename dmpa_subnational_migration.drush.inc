<?php

use \Drupal\taxonomy\Entity\Term;

/**
 * Implements hook_drush_command().
 */
function dmpa_subnational_migration_drush_command() {
  $items = [];

  $items['dmpa_subnational_migration_level_one'] = [
    'description' => 'Migrates sub-national level one',
    'aliases' => ['lvl_one'],
  ];

  return $items;
}

/**
 * Drush command 'drush lvl_one' implementation. org_units.csv should be located
 * in the module folder
 */
function drush_dmpa_subnational_migration_level_one() {
  drush_print('Level one');
  $module_path = drupal_get_path('module', 'dmpa_subnational_migration');
  // points to the org_units.csv file located in this module since the @module_path
  // variable refers to this module
  $file = fopen($module_path . '/org_units.csv', "r");

  $r = 0; // row counter with 0 being first row
  while (!feof($file)) {
    $row = fgetcsv($file);

    if ($r == 0) {
      $r++;
      continue;
    }
    //if the term in column 1 does not exist, then create it
    if (count(checkTermExists($row[1])) == 0) {
      //print('create me');
      createTerm($row, 1);

      if (!empty($row[2])) {
        $tid = checkTermExists($row[1])[0]->tid;
        createTerm($row, 2, $tid);
      }
    }
    else {
      if (!empty($row[2])) {
        //print('col: '.$row[2]."\n");
        $tid = checkTermExists($row[1])[0]->tid;
        createTerm($row, 2, $tid);
      }
    }
  }
  fclose($file);
}

function getDatabaseConnection($key = 'default') {
  \Drupal\Core\Database\Database::setActiveConnection($key);
  $db = \Drupal\Core\Database\Database::getConnection();

  return $db;
}

/**
 * @param $row
 * row in csv file
 *
 * @param $column
 *
 * column in csv file
 * @param null $tid
 *
 * term id
 * @param string $vid
 *
 * vocabulary id
 */
function createTerm($row, $column, $tid = NULL, $vid = 'subnational_level') {
  $countryId = checkTermExists($row[0])[0]->tid;
  $parameters = [
    'name' => $row[$column],
    'vid' => $vid,
    'description' => $row[$column],
    'field_country' => ['target_id' => $countryId],
  ];

  if ($column == 2) {
    print("\nset parent to tid: " . $tid);
    $parameters['parent'] = ['target_id' => $tid];
  }
  Term::create($parameters)->save();
}

/**
 * @param string $name
 *
 * Term name
 *
 * @return mixed
 *
 * Returns resultset
 */
function checkTermExists($name) {
  $db = getDatabaseConnection();
  $query = $db->select('taxonomy_term_field_data', 'fd')
    ->fields('fd', ['tid', 'name'])->condition('name', $name);
  return $query->execute()->fetchAll();
}
