const observer = new IntersectionObserver((entries) => {
  entries.forEach((entry) => {
    if (entry.isIntersecting) entry.target.classList.add("is-visible");
  });
}, { threshold: 0.12 });

document.querySelectorAll(".feature-card, .gallery-grid, .step, .topic-row").forEach((item) => {
  item.classList.add("reveal");
  observer.observe(item);
});
