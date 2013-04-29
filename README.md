###Paraweb

Barebones static website generator with online editing, git integration and cgi-helper for contact form.

You can edit your site locally (markdown) and run 'make publish', or you can
add a hook to your git repository, and let your site build itself after you 
commit changed markdown.

You can also double click on your site and edit it in place, and submit to git
from there. How cool is that?

Finally, if you use github, you can even use their builtin editor!

#### Building a nice looking website with paraweb

If you run 'make', something reasonable comes out. Suggested invocation: make && google-chrome index.html

The following files control layout:

 + header.html  
   This sets up most of the CSS, fonts etc.
 + *.md  
   This is your own contribution
 + endcontent.html  
   This provides the closing HTML for the content section
 + *menu.html  
   Sidebar menu
 + footer.html  
   Bottom of your site 
 + end.html  
   HTML closing statements

Meanwhile, 'website.conf' determines which pages to build, and what menus to apply to your content.
'website.conf.local' finally has details on where you want to publish your site.

To hook up the contact form and editing, install cgi-bin/{changer,mailer} to http://yourwebsite/cgi-bin/, and
make sure they work. 

Website.conf.local configures where the cgi-bin scripts read their configuration from.

#### Editing locally
From a git checkout you can run 'make publish' which will build your website and rsync it to the live webserver.

However, if multiple people do this, changes might be overwritten on the live webserver. Rsync does not 'merge'.

#### Cgi-bin gateway
There is a cgi-bin gateway that powers the contact form. 
There is a second version of the gateway that allows access to the 'staged' version and triggers rebuilds.

The cgi-bin scripts read a .ini configuration file from the full filename configured
in website.conf.local. A sample is in the distribution.

#### Staged version of the website
This is a version of the website that can be edited via the browser. It too gets rebuilt on a trigger, and will
merge git changes.

In addition to allowing editing, this version of the website has buttons:
'diff' 'revert' 'push'.

'diff' shows all changes of the staged version of the website compared to git HEAD. 
'revert' reverts all local changes
'push' actually pushes the changes, triggering a rebuild of both staged and main website.

The stage version of the website is hosted by the 'changer' cgi-bin script.

