
###### Part of paraweb. Configure your website in website.conf please! ############

ALLPAGES = ${PAGES} sitemap.txt status-404.html sitemap.html login.html

all: ${ALLPAGES} 

-include website.conf

sitemap.md: makesitemap $(filter-out sitemap.html status-404.html, $(ALLPAGES))
	./makesitemap $(filter-out sitemap.html status-404.html, $(ALLPAGES)) > sitemap.md

sitemap.txt: $(filter-out login.html sitemap.html status-404.html, $(ALLPAGES))
	for name in $(filter-out sitemap.html status-404.html, $(ALLPAGES)) ; do echo ${BASEURL}$$name; done > $@
	
clean: 
	rm -f ${ALLPAGES} sitemap.md 404.html *~ 

todistribute = ${ALLPAGES} status-404.html 404.html css js img resources favicon.ico sitemap.txt ${PROOFFILES} 
stagedistribute = *.md index.content epiceditor jsdiff

ifeq ($(WWWTYPE),live)
EDITOR=
publish: all
	rsync -rz --no-p --no-g --chmod=ug=rwX ${todistribute} ${WWWLIVE}
else
EDITOR=editor.html
publish: all 
	rsync -rz --no-p --no-g --chmod=ug=rwX ${todistribute} ${stagedistribute} ${WWWSTAGE}
endif	

.SECONDEXPANSION:     
%.html: %.content footer.html endcontent.html header.html ${EDITOR}  end.html $(or ${$*_MENU}, ${default_MENU})
	cat header.html $< endcontent.html $(or ${$*_MENU}, ${default_MENU}) footer.html ${EDITOR} end.html > $@

%.content: %.md
	markdown $< > $@

status-404.html: 404.html
	sed 's|<title>|<base href="${BASEURL}"><title>|' < $< > $@
