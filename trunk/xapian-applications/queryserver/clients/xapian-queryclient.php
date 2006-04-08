<?

class Hit {
    var $rank;

    function Hit() {
        $this->rank = -1;
    }
}

class Result {
    var $results;

    function Result() {
        $this->hits = array();
    }
}

class Enquiry {
    var $queryserver_socket;

    var $dbname;
    var $query;
    var $filterlist;
    var $collapsekey;
    
    function Enquiry($socket) {
        $this->queryserver_socket = $socket;
        $this->dbname = 'default';
        $this->query = '';
        $this->filterlist = '';
        $this->collapsekey = false;
    }

    function setCollapse($keyno) {
        $this->collapsekey = $keyno;
    }

    function setDbName($dbname) {
        $this->dbname = $dbname;
    }

    function setQuery($query) {
        $this->query = $query;
    }

    function addFilter($prefix, $filter) {
        $this->filterlist .= "prefix:" . base64_encode($prefix) . "\n";
        $this->filterlist .= "filter:" . base64_encode($filter) . "\n";
    }

    function _store_keyval(&$hit, &$result, $key, $value) {
        if ($hit == false) {
            // Add entry to result object.
            if ($key == 'firstitem' ||
                $key == 'matches_lower_bound' ||
                $key == 'matches_estimated' ||
                $key == 'matches_upper_bound' ||
                $key == 'items')
            {
                $result->$key = $value;
            }
        } else {
            if ($key == 'docid' ||
                $key == 'rank' ||
                $key == 'percent')
            {
                $hit->$key = $value;
            } else if ($key == 'data') {
                $data = base64_decode($value);
                $data = explode("\n", trim($data));
                foreach ($data as $dataitem) {
                    $eq = strpos($dataitem, '=');
                    $key = substr($dataitem, 0, $eq);
                    $value = substr($dataitem, $eq + 1);
                    $hit->data->$key = $value;
                }
            }
        }
    }

    function perform($firstdoc, $maxitems)
    {
        $socket = socket_create(AF_UNIX, SOCK_STREAM, 0);
        if ($socket == false) {
            return false;
        }

        if (!socket_connect($socket, $this->queryserver_socket)) {
            return false;
        }

        $request  = 'db:' . base64_encode($this->dbname) . "\n";
        $request .= 'query:' . trim(chunk_split(base64_encode($this->query)));
        $request .= "\n";
        $request .= $this->filterlist;
        $request .= "firstdoc:$firstdoc\n";
        $request .= "maxitems:$maxitems\n";
        if ($this->collapsekey != false) {
            $request .= "collapsekey:$this->collapsekey\n";
	}
        $request .= "\n";
        socket_write($socket, $request);
        //FIXME - check for errors from socket_write

        // Read the return message
        $lines = '';
        $newbit = '';
        do {
            $newbit = socket_read($socket, 10000, PHP_BINARY_READ);
            if ($newbit == '') {
                break;
            }
            $lines .= $newbit;
        } while($newbit != '');

        $lines = explode("\n", trim($lines));

        $result = new Result;
        $hit = false;
        $key = false;
        $value = false;
        foreach ($lines as $line) {
            if ($line == '#') {
                if ($key != false) {
                    $this->_store_keyval($hit, $result, $key, $value);
                    $key = false;
                }
                if ($hit != false) {
                    $result->hits[$hit->rank] = $hit;
                }
                $hit = new Hit;
                continue;
            }

            $colon = strpos($line, ':');
            if ($colon != false) {
                if ($key != false) {
                    $this->_store_keyval($hit, $result, $key, $value);
                }
                $key = substr($line, 0, $colon);
                $value = substr($line, $colon + 1);
            } else {
                $value .= "\n" . $line;
            }
        }
        if ($key != false) {
            $this->_store_keyval($hit, $result, $key, $value);
            $key = false;
        }

        return $result;
    }
}

?>
