### Contact form
To contact us, please use this contact form.

<form id="contactform">
  <fieldset>
    <div class="row">
      <div class="large-8 columns">
        <label>Your name</label>
        <input type="text" name="name" placeholder="Your name" onblur="testForm()">
      </div>
     <div class="large-4 columns">
      <label for="customDropdown1">Subject</label>
      <select id="customDropdown1" name="subject" class="medium" onblur="testForm()">
        <option DISABLED>Select a subject</option>
        <option>Request a quote</option>
        <option>Urgent need for support</option>
	<option>Report security issue</option>
        <option>Professional services</option>
        <option>Media query</option>
        <option>General question</option>
      </select>
    </div>
    </div>

    <div class="row">
      <div class="large-4 columns">
        <label>Organization</label>
        <input type="text" name="organization" placeholder="Organization" onblur="testForm()">
      </div>
      <div class="large-4 columns">
        <label>Email Address</label>
        <input type="text" class="" placeholder="Email address" name="email" onblur="testForm()">
      </div>
      <div class="large-4 columns">
          <label>Phone Number</label>
          <input type="text" name="phone" placeholder="Phone number" onblur="testForm()">
      </div>
    </div>

    <div class="row">
      <div class="large-12 columns">
        <label>Your query</label>
        <textarea rows="15" cols="78" name="query" placeholder="Your query"></textarea>
      </div>
 
    </div>
    <div class="row">
	<div class="large-12 columns" style="text-align: right" name="status">
		<a href="javascript:void(0)" class="disabled button" name="submit" onClick="submitForm()">Submit</a>
	</div>
    </div>
  </fieldset>
</form>


<script type="text/javascript">
function testForm()
{
	var emailOk=$("#contactform [name=email]").val().match(".+@.+");
	var nameOk=$("#contactform [name=name]").val()!="";

	if(emailOk)
            $("#contactform [name=email]").removeClass("error");
	else
            $("#contactform [name=email]").addClass("error");

	if(nameOk)
		$("#contactform [name=name]").removeClass("error");
	else
		$("#contactform [name=name]").addClass("error");

	if(emailOk && nameOk)
		$("#contactform [name=submit]").removeClass("disabled");
	else
		$("#contactform [name=submit]").addClass("disabled");
}

function submitForm()
{
	if($("#contactform [name=submit]").hasClass("disabled"))
		return;

	var story={};
	story.action = "email";
	story.name = $("#contactform [name=name]").val();
 	story.organization=$("#contactform [name=organization]").val();
	story.email=$("#contactform [name=email]").val();
	story.query=$("#contactform [name=query]").val();
	story.phone=$("#contactform [name=phone]").val();
	story.subject=$("#contactform [name=subject]").val();
	story.obfuscated = 3551233*3551233;
	$("#contactform [name=submit]").removeClass("button");
	$("#contactform [name=status]").html("Sending");
	repeater = setInterval(function() 
		{ 
			var txt = $("#contactform [name=status]").html();
			if(txt.length < 11)
				txt += ".";
			else 
				txt = "Sending";
			$("#contactform [name=status]").html(txt);
		}, 500);

	$.post("/cgi-bin/mailer", JSON.stringify(story), 
		function(data) { 

			$("#contactform [name=status]").html(data["status"]);
			clearInterval(repeater);
		}, 
		"json");

	return false;
}

$(document).ready(function() {
	testForm();
});
</script>
