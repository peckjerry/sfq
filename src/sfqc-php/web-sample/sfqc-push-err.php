<?php
	require 'common.inc';
	printHeaders();
?>
<html>
<body>
<?php include_once("analyticstracking.php") ?>
<a href="https://github.com/cbh34680/sfq/blob/master/src/sfqc-php/web-sample/<?= basename(__FILE__) ?>" target="_blank">source</a><br />
<?php
try
{
	$reqv = [
		'execpath' =>'ls',
		'execargs' => implode("\t", ['-l' , '/home/NotFound/']),
		'soutpath' => '@',
		'serrpath' => '@',
	];

	$sfqc = SFQueue::newClient([ 'quename' => 'webque-1' ]);
	$uuid = $sfqc->push_binary($reqv);

	if ($uuid)
	{
		if (preg_match_all('/../', implode(explode('-', $uuid)), $matches))
		{
			$dir = '/var/tmp/webque-1/logs/exec/' . implode('/', $matches[0]);
		}
	}
}
catch (Exception $ex)
{
}

?>
<?php if (isset($ex)) : ?>
<hr />
CODE: <?= $ex->getCode() ?><br />
MESG: <?= $ex->getMessage() ?><br />
<?php endif ?>
<hr />
<pre>
<?php if (isset($reqv)) { var_dump($reqv); } ?>
<?= var_dump(@$uuid) ?>
</pre>
<?php if (isset($dir)) : ?>
<hr />
<i><a href='cat.php?uuid=<?= $uuid ?>&file=id.txt'  target='_blank'>see</a></i>: <?= $dir ?>/id.txt<br />
<i><a href='cat.php?uuid=<?= $uuid ?>&file=std.out' target='_blank'>see</a></i>: <?= $dir ?>/std.out<br />
<i><a href='cat.php?uuid=<?= $uuid ?>&file=std.err' target='_blank'>see</a></i>: <?= $dir ?>/std.err<br />
<i><a href='cat.php?uuid=<?= $uuid ?>&file=rc.txt'  target='_blank'>see</a></i>: <?= $dir ?>/rc.txt<br />
<?php endif ?>
</body>
</html>

