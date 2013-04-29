### Login

<script src="js/purl.js"></script>

<form action="javascript:doLogin()">
	<fieldset>
		<label>Username</label><input type="text" id="username"/>
		<label>Password</label><input type="password" id="password"/>
		<input type="submit" class="button" value="Login"/>
	</fieldset>
</form>

<script>

function doLogin()
{
	var keys={};
	keys.user = $("#username").val();
	keys.password = $("#password").val();
	$.get("cgi-bin/changer?action=login", 
		keys, 
		function(e) { 
			console.log("Got "+e); 
			if(e.status=="Ok")
				window.location=purl().param("goal");
			else 
				alert(e.status);
		}, "json"
	);
}

</script>