/*gittalk id must be less than 50*/
var id = window.location.pathname.split('/')[window.location.pathname.split('/').length - 1];
if (id.length > 50) {
	id = id.split('_').join('')
}

var gitalk = new Gitalk({
    "clientID": "7bf1e91eaff35fd004dc",
    "clientSecret": "d151c7bc85d544dafe2c0de75e3a174623229a3a",
    "repo": "wangwei1237.github.io",
    "owner": "wangwei1237",
    "admin": ["wangwei1237"],
    "id": id, 
    "distractionFreeMode": false
});

gitalk.render("gitalk-container");
