<?php
	require 'common.inc';
	printHeaders();
?>
<html>
<body>
<?php
try
{
	$sfqc = SFQueue::newClient('webque-0');
	$resp = $sfqc->pop();
}
catch (Exception $ex)
{
}

?>
<? if (isset($ex)) : ?>
<hr />
CODE: <?= $ex->getCode() ?><br />
MESG: <?= $ex->getMessage() ?><br />
<? endif ?>
<hr />
<pre>
<? if (isset($reqv)) { var_dump($reqv); } ?>
<?= var_dump(@$resp) ?>
</pre>
</body>
</html>