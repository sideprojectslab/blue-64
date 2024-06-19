asciidoctor-pdf -r asciidoctor-mathematical -a pdf-theme=resources/theme.yml -a env-pdf README.adoc -o ../README.pdf
rm -rf stem*
asciidoctor README.adoc -o ../README.html
#asciidoctor -b docbook README.adoc
#pandoc -f docbook -t markdown_strict README.xml -o ../README.md
#rm -rf README.xml
