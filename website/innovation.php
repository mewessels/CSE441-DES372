<!DOCTYPE html>
<head>
    <link rel="stylesheet" href="styles/styles.css"/>

</head>
<body class="innovation">
    <div class="question-container">
        <div class="question">
            <h3 class="subtitle">WHAT IS IT ABOUT INNOVATION DRIVES YOU?</h3>
        </div>
        <div class="response">
            <form method="post">
                <input type="text" name="quote">
                <input type="submit" value="SUBMIT">
            </form>
            <?php
              if (isset($_POST['submit'])) {
                  DEFINE('DB_USERNAME', 'root');
                  DEFINE('DB_PASSWORD', 'root');
                  DEFINE('DB_HOST', 'localhost');
                  DEFINE('DB_DATABASE', 'interface_design');

                  $mysqli = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_DATABASE);

                  if (mysqli_connect_error()) {
                    die('Connect Error ('.mysqli_connect_errno().') '.mysqli_connect_error());
                  }

                  echo 'Connected successfully.';

                  $sql = "INSERT INTO innovation (id,quote) VALUES ('1','$_POST[quote]')";
                  if (!mysqli_query($mysqli, $sql)) {
                      echo 'Not inserted';
                  } else {
                      echo 'inserted succesfully';
                  }
                  $mysqli->close();
              }
              ?>
        </div>
    </div>

</body>
</html>
