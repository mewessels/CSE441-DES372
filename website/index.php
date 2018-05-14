<!DOCTYPE html>
<html lang="en">
<head>
    <link rel="stylesheet" href="styles/styles.css">
    <meta charset="UTF-8">
    <?php
      DEFINE('DB_USERNAME', 'root');
      DEFINE('DB_PASSWORD', 'root');
      DEFINE('DB_HOST', 'localhost');
      DEFINE('DB_DATABASE', 'interface_design');

      $mysqli = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_DATABASE);

      if (mysqli_connect_error()) {
        die('Connect Error ('.mysqli_connect_errno().') '.mysqli_connect_error());
      }

      echo 'Connected successfully.';

      $mysqli->close();
?>
</head>
<body class="main">
    <div class="full-width-text title">
        <h1>WHAT DRIVES YOU?</h1>
    </div>
    <div class="container">
        <div class="item">
            <h3 class="subtitle">MONEY</h3>
            <div class="money-bar"></div>
        </div>
        <div class="item">
            <h3 class="subtitle">HAPPINESS</h3>
            <div class="happiness-bar"></div>
        </div>
        <a class="item" href="./innovation.php">
            <div class="item">
                <h3 class="subtitle">INNOVATION</h3>
                <div class="innovation-bar"></div>
            </div>
        </a>
        <div class="item">
            <h3 class="subtitle">IMPACT</h3>
            <div class="impact-bar"></div>
        </div>
        <div class="item">
            <h3 class="subtitle">LOVE</h3>
            <div class="love-bar"></div>
        </div>
        <div class="item">
            <h3 class="subtitle">CREATIVITY</h3>
            <div class="creativity-bar"></div>
        </div>
    </div>


</body>

</html>