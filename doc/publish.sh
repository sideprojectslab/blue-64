#asciidoctor-pdf -r asciidoctor-mathematical -a pdf-theme=themes/theme.yml -a env-pdf README.adoc
asciidoctor README.adoc -o ../README.html
rm -rf stem*
