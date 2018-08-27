var grow=0;

function doForWsItems(p,fn){
	var owner=document.getElementById(p[1]);
	var n=(p.length-2)/2;
	for(i=0;i<n;i++) fn(owner,p[(i*2)+2],p[(i*2)+3]);
}
function dynup(c,search,nv){
	var childs = c.childNodes;
	var len = childs.length, i = -1;
	if(++i < len) do {
		var item=childs[i].firstChild;
		if(item){
			if(item.innerHTML==search){
				item.setAttribute("class","sm");
				item.nextElementSibling.innerHTML=nv;
				setTimeout(function(x){	x.setAttribute("class","sl"); }, 350,item);
				break;
			}
		} //else console.log("a tad premature "+search);
	} while(++i < len);
//	if(i==len) console.log("********* shes not there! '"+search+"'");
}

function decodeWsMsg(msg){
	var p=msg.split("|");
	switch(p[0]){
		case 'grf': // hate this
			switch(p[1]){
				case 'ga':
					ga=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
				case 'gh':
					gh=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
				case 'gq':
					gq=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
				case 'gp':
					gp=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
			}
			break;
		case 'chk':
			document.getElementById("titles").children[p[1]].setAttribute("class","chk");
			break;
		case 'beat':
			var heart=document.getElementById("hb");
			heart.setAttribute("class","cmd big");
			setTimeout(function(){
				heart.setAttribute("class","cmd");
				},250);
			if((grow%60)===0){
				var grass=document.getElementById("grass");
				var pc=parseFloat(window.getComputedStyle(grass).backgroundSize.replace(/%$/,''));
				if(pc!=100) pc+=0.25;
				else pc=0;
				console.log("just a minute! pc now="+pc);
				grass.style.backgroundSize=pc+"%";
			}
			grow++;
			break;
		case 'vis':
			document.getElementById(p[1]).style.visibility="visible";
			break;
		case 'show':
			document.getElementById(p[1]).style.display="block";
			break;
		case 'rbc': // show by class
			var rbcs=document.getElementsByClassName(p[1]);
			var i = rbcs.length;	
			while (i--) rbcs[i].classList.remove(p[2]);
			break;
		case 'tool':
			var o=document.getElementById(p[0]);
			var inp=document.createElement("input");
			inp.value=p[2];
			inp.name=p[1];
			inp.id=p[1];
			inp.onblur=vc;
			inp.required=true;
			var divlt=document.createElement("div");
			divlt.className="lt";
			divlt.appendChild(inp);
			var divdk=document.createElement("div");
			divdk.className="dk";
			divdk.innerHTML=p[1];
			var divsb=document.createElement("div");
			divsb.className="sb";										
			divsb.appendChild(divdk);
			divsb.appendChild(divlt);				
			o.appendChild(divsb);
			break;
		case 'aso': // add select options
			doForWsItems(p,function(o,v1,v2){	
				var selopt=document.createElement("option");
				selopt.value=v1;
				selopt.innerHTML=v2;
				o.appendChild(selopt);
			});
			break;
		case 'sso':
			var selem=document.getElementById(p[1]);
			var selopts=selem.childNodes;
			for(i=0;i<selopts.length;i++){
				if(selopts[i].value==p[2]){
					selopts[i].selected=true;
					break;
				}
			}
			break;
		case 'cbi':  // class by id: p[1]=id p[2]=value
			document.getElementById(p[1]).setAttribute("class",p[2]);
			break;
		case 'ibi':  // innerHTML by id: p[1]=id p[2]=value
			var n=(p.length-1)/2;
			for(i=0;i<n;i++){
				var idn=p[(i*2)+1];
				var d=document.getElementById(idn);
				if(d!==null) {
					var value=p[(i*2)+2];
					if(d.tagName=="INPUT") d.value=value;
					else d.innerHTML=value;
					// a better way?
					if(idn=="heap")	gh.plot(value);
					else if(idn=="Q") gq.plot(value);
					else if(idn=="sps")	gp.plot(value);
					else if(idn=="adc")	ga.plot(value);									
				}
			}
			break;
		case 'clr':
			var clr=document.getElementById(p[1]);
			while (clr.firstChild) clr.removeChild(clr.firstChild);
			break;
		case 'tsel':
			var grf=document.getElementById("tsel");
			var act=p[1];
			switch(act){
				case "add":
//					console.log("DYNUP ADD "+p[2]+" "+p[3]);
					var divlt=document.createElement("div");
					divlt.className="lt";
					divlt.innerHTML=p[3];
					var divdk=document.createElement("div");
					divdk.className="sl";
					divdk.innerHTML=p[2];
					var divsb=document.createElement("div");
					divsb.className="sb";										
					divsb.appendChild(divdk);
					divsb.appendChild(divlt);				
					grf.appendChild(divsb);
					break;
				case "dupc":
					//console.log("DYNUP C "+p[2]+" "+p[3]);
					dynup(grf,p[2],p[3]);
					break;
				case "dups":
					//console.log("DYNUP S "+p[2]+" "+p[3]);
					dynup(grf,"source: "+p[2],p[3]);
					break;
				default:
					//console.log("dodgy grf "+act);
			}
			break;
		case 'rfs': // DIRTY HACK this really shouldn't be here...needs shifting
			var ibf=document.getElementById("last");
			doForWsItems(p,function(o,v1,v2){		
				var divlhs=document.createElement("div");
				divlhs.className="lhs";
				divlhs.innerHTML=v1;
				var divled=document.createElement("div");
				divled.className="output ld led-"+(v2=="0" ? "red":"green");
				divled.id=v1;
				divled.onclick=rfClick;
				var divrhs=document.createElement("div");
				divrhs.className="rhs";										
				divrhs.appendChild(divled);
				o.insertBefore(divlhs,ibf);
				o.insertBefore(divrhs,ibf);
			});
			break;
		default:
			console.log("dodgy ws cmd "+p[0]);
	}	
}