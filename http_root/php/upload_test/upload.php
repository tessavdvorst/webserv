<?php
$target_dir = "img/";
$target_file = $target_dir . "gopher++.png";
$uploadOk = 1;

if(isset($_POST["submit"])) {
//	echo "HAAP" . $_FILES["imgfile"]["tmp_name"] . "PAAH";
	$check = getimagesize($_FILES["imgfile"]["tmp_name"]);
	if ($check !== false) {
		echo "Thanks for uploading your " . $check["mime"] . " image to GopherCorp!";
	} else {
		echo "Huh? That's not an image...";
		$uploadOk = 0;
	}

	if ($_FILES["imgfile"]["size"] > 5000000) {
		echo "That's a pretty big file, please pick a smaller one...";
		$uploadOk = 0;
	}

	if (strtolower(pathinfo(basename($_FILES["imgfile"]["name"]), PATHINFO_EXTENSION)) != "png") {
		echo "Sorry, corporate only allows PNG files.";
		$uploadOk = 0;
	}
} else {
	echo "You submitted an empty form, silly!";
	$uploadOk = 0;
}

if ($uploadOk == 0) {
	echo "As we're sure you'll understand, your file wasn't allowed.";
} else {
	if (move_uploaded_file($_FILES["imgfile"]["tmp_name"], "img/gopher++.png")) {
		echo "You have successfully uploaded a new Gopher image!";
	} else {
		echo "There was an error uploading your new Gopher image...";
	}
}

if (isset($_POST["other_data"])) {
	echo "Your comment - " . htmlspecialchars($_POST["other_data"]) . " - is now part of our official slogan, congratulations!";
}
?>
