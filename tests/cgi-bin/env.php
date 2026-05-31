<?php
$env_vars = getenv();
foreach ($env_vars as $key => $value) {
    echo $key . " = " . $value . "\n";
}
?>

