main(n){scanf("%d",&n);if(n%10>6)n+=10-n%10;printf("%d\n",n/10*100+n%10*15);exit(0);}