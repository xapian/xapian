<?
include('xapian-queryclient.php');
$enquiry = new Enquiry('/tmp/xapian-queryserver-socket');
$enquiry->setDbName('db');
$enquiry->setQuery('query');
$result = $enquiry->perform(0, 10);
if ($result != null) {
   print "First item returned is of rank: " .
           $result->firstitem . " <br>\n";
   print "Number of items returned is: " .
           $result->items . " <br>\n";
   print "There were at least: " .
           $result->matches_lower_bound . " hits.<br>\n";
   print "There were approximately: " .
           $result->matches_estimated . " hits.<br>\n";
   print "There were no more than: " .
           $result->matches_upper_bound . " hits.<br>\n";
   foreach ($result->hits as $hit) {
     print "Hit at rank " . $hit->rank .
           ", score " . $hit->percent . "%" .
           " is " . $hit->data->type . "_" . $hit->data->id .
           "<br>\n";
   }
}
?>
