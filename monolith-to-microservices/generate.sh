#!/bin/sh

HEXO=/usr/local/bin/hexo

$HEXO clean
$HEXO generate

cd public && \
rm -rf digital-video-concept && \
cp -r /$HOME/Documents/Project/webroot/book/video digital-video-concept && \
rm -rf monolith-to-microservices && \
cp -r /$HOME/Documents/Project/webroot/book/m2m monolith-to-microservices && \
rm -rf discovery-the-unpredictable-risk && \
cp -r /$HOME/Documents/Project/webroot/book/dr discovery-the-unpredictable-risk

book1="\ \ <url><loc>https://wangwei1237.github.io/digital-video-concept/</loc><lastmod>2020-06-17T02:55:11.212Z</lastmod></url>"
book2="\ \ <url><loc>https://wangwei1237.github.io/monolith-to-microservices/</loc><lastmod>2020-06-17T02:55:11.212Z</lastmod></url>"

sed -i '' "4 i\ 
${book1} 
" sitemap.xml

sed -i '' "4 i\ 
${book2} 
" sitemap.xml

cp sitemap.xml bd_sitemap.xml && sed -i '' 's/github/gitee/g' bd_sitemap.xml
cp monolith-to-microservices/sitemap.xml m2m_sitemap.xml
cp m2m_sitemap.xml m2m_e_sitemap.xml && sed -i '' 's/gitee/github/g' m2m_e_sitemap.xml

cd .. 
cp -r docs/* public/shares

