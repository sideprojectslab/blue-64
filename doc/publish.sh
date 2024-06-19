#asciidoctor-pdf -r asciidoctor-mathematical -a pdf-theme=themes/theme.yml -a env-pdf README.adoc
asciidoctor -b docbook README.adoc
pandoc -f docbook -t markdown_strict README.xml -o ../README.md
rm -rf README.xml
rm -rf stem*
